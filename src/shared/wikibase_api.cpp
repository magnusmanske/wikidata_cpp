#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include "wikibase.h"
#include <curl/curl.h>


struct CURLMemoryStruct {
	char *memory;
	size_t size;
};


json WikibaseAPI::getSiteInfo () {
	loadSiteInfo() ;
	return site_info.at("query").at("general") ;
}

string WikibaseAPI::urlEncode(const string &value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}

size_t WikibaseAPI::WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct CURLMemoryStruct *mem = (struct CURLMemoryStruct *)userp;

	mem->memory = (char*) realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) throw "WikibaseAPI::WriteMemoryCallback: out of memory" ;

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

json WikibaseAPI::loadJSONfromURL ( string url , string post_parameters , uint8_t depth ) {
	json j ;
	CURL *curl;
	curl = curl_easy_init();
	if ( !curl ) throw "WikibaseAPI::loadJSONfromURL: CURL does not init" ;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	if ( !post_parameters.empty() ) {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_parameters.c_str());
	}

	struct CURLMemoryStruct chunk;
	chunk.memory = (char*) malloc(1);  /* will be grown as needed by the realloc above */ 
	chunk.size = 0;    /* no data at this point */ 

	//curl_easy_setopt(curl, CURLOPT_VERBOSE , 1L); // DEBUG	
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirect; paranoia
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, agent_name.c_str());


	CURLcode res ;
	try {
		res = curl_easy_perform(curl);
	} catch ( const char *s ) {
		throw s ;
	} catch ( ... ) {
		throw "Problem with curl_easy_perform" ;
	}


	if ( !(res == CURLE_OK && chunk.size > 0 && chunk.memory) && depth < repeat_curl ) {
		std::this_thread::sleep_for(std::chrono::seconds(sleep_curl_repeat_seconds));
		cout << "Iteration " << (depth+1) << endl ;
		return loadJSONfromURL ( url , post_parameters , depth+1 ) ;
	}

	if (res != CURLE_OK) throw "WikibaseAPI::loadJSONfromCURL: CURL not OK" ;
	if ( chunk.size == 0 ) throw "WikibaseAPI::loadJSONfromCURL: Chunk size 0" ;
	if ( !chunk.memory ) throw "WikibaseAPI::loadJSONfromCURL: No memory" ;

	char *text = chunk.memory ;
	curl_easy_cleanup(curl);

	try {
		j = json::parse ( text ) ;
	} catch ( json::exception& e ) {
		free ( text ) ;
		throw e ;
	}
	
	free ( text ) ;
	return j ;
}

void WikibaseAPI::loadSiteInfo () {
	if ( is_site_info_loaded ) return ;
	site_info = runQuery ( {
		{"action","query"},
		{"meta","siteinfo"},
		{"siprop","general"}
	} ) ;
	is_site_info_loaded = true ;
}

json WikibaseAPI::runQuery ( json query ) {
	string url ( api_path );
	string parameters = "format=json" ;
	for (json::iterator parameter = query.begin(); parameter != query.end(); ++parameter) {
		parameters += "&" + urlEncode(parameter.key()) + "=" + urlEncode(parameter.value()) ;
	}

	if ( true ) { // use GET
		url += "?" + parameters ;
		parameters.clear() ;
	}

	json ret ;
	try {
		ret = loadJSONfromURL ( url , parameters ) ;
	} catch ( const char *s ) {
		cerr << "WikibaseAPI::runQuery: On URL " << url << endl << "exception: " << s << endl ;
		exit(1) ;
	} catch ( ... ) {
		cerr << "WikibaseAPI::runQuery: On URL " << url << " exception " << endl ;
		exit(1) ;
	}
	return ret ;
}

#ifndef __wikibase_H__
#define __wikibase_H__

#include <string>
#include "../vendor/json.hpp" // https://github.com/nlohmann/json

using namespace std ;
using json = nlohmann::json;

typedef char WikibaseEntityType ;
typedef string WikibaseID ;
typedef vector <WikibaseID> WikibaseEntityList ;

class WikibaseException : public std::exception {
public:
    explicit WikibaseException(const std::string& message, const std::string method_name = "" ): message(message),method_name(method_name) {}
    virtual ~WikibaseException() throw (){}
    virtual const char* what() const throw () {
    	string ret = message ;
    	if ( !method_name.empty() ) ret += " [in " + method_name + "]" ;
    	return ret.c_str();
    }
    string message , method_name ;
} ;

class WikibaseAPI {
public:
	WikibaseAPI () {}
	WikibaseAPI ( string api_path ) : api_path(api_path) {}
	json runQuery ( json query ) ;
	json getSiteInfo () ;

protected:
	json loadJSONfromURL ( string url , string post_parameters = "" , uint8_t depth = 0 ) ;
	static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) ;
	string urlEncode ( const string &value ) ;
	void loadSiteInfo() ;

	string api_path = "https://www.wikidata.org/w/api.php" ;
	string agent_name = "Wikibase-cpp/1.0" ;
	json site_info ;
	bool is_site_info_loaded = false ;
	int8_t repeat_curl = 1 ;
	uint8_t sleep_curl_repeat_seconds = 2 ;
} ;

class WikibaseEntity {
public:
	WikibaseEntity () {} ;
	WikibaseEntity ( WikibaseID id ) : id(id) {}
	WikibaseEntity ( WikibaseID id , std::shared_ptr<WikibaseAPI> api ) ;
	WikibaseEntity ( json &j , std::shared_ptr<WikibaseAPI> api ) ;
	void loadDataFromApi ( std::shared_ptr<WikibaseAPI> _api ) ;

	bool isInitialized() { return !id.empty() ; }
	bool isDataLoaded() { return !j.empty() ; }
	static bool isValidID ( WikibaseID id ) ;

	static WikibaseEntityType getType ( const WikibaseID &id ) ;
	WikibaseEntityType getType () { return getType(id) ; }
	string getEntityURL() ;
	string getWebURL() ;

	// Property access
	string getEntityType () { return j.at("type") ; }
	int32_t getPageID () { return j.at("pageid") ; }
	int32_t getLastRevisionID () { return j.at("lastrevid") ; }
	string getPageTitle() { return j.at("title") ; }
	string getLastModificationDate() { return j.at("modified") ; }

	bool hasLabelInLanguage ( string language_code ) ;
	bool hasAliasesInLanguage ( string language_code ) ;
	bool hasDescriptionInLanguage ( string language_code ) ;
	bool hasSitelinkToWiki ( string wiki ) ;
	bool hasBadgesInWiki ( string wiki ) ;
	string getLabelInLanguage ( string language_code ) ;
	vector <string> getAliasesInLanguage ( string language_code ) ;
	string getDescriptionInLanguage ( string language_code ) ;
	string getSitelinkToWiki ( string wiki ) ;
	vector <string> getBadgesInWiki ( string wiki ) ;

	bool hasClaimsForProperty ( WikibaseID property ) ;
	json getClaimsForProperty ( WikibaseID property ) ;
	vector <WikibaseID> getTargetItemsFromClaims ( const json &claims ) ;
	vector <WikibaseID> getTargetItemsForProperty ( WikibaseID property ) { return getTargetItemsFromClaims ( getClaimsForProperty ( property ) ) ; }
	vector <string> getStringsFromClaims ( const json &claims ) ;
	vector <string> getStringsForProperty ( WikibaseID property ) { return getStringsFromClaims ( getClaimsForProperty ( property ) ) ; }


	// TODO: claims

protected:
	WikibaseID id ;
	json j ;
	std::shared_ptr<WikibaseAPI> api ;
} ;

class WikibaseEntities {
public:
	WikibaseEntities () : api(std::make_shared<WikibaseAPI>()) {}
	WikibaseEntities ( string api_path ) : api(std::make_shared<WikibaseAPI>(api_path)) {}
	void loadEntities ( const vector <WikibaseID> &load_entities ) ;
	size_t size() { return entities.size() ; }
	bool isEntityLoaded ( const WikibaseID &id ) ;
	WikibaseEntity getEntity ( WikibaseID id , bool autoload = false ) ;

protected:
	string joinEntityIDs ( const WikibaseEntityList &ids , const string &separator ) ;
	map <WikibaseID,WikibaseEntity> entities ;
	std::shared_ptr<WikibaseAPI> api ;
} ;

#endif
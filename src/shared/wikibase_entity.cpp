#include "wikibase.h"
#include <stdexcept>
#include <iostream>
#include <regex>


WikibaseEntity::WikibaseEntity ( json &j , std::shared_ptr<WikibaseAPI> api ) : j(j),api(api) {
	id = j.at("id") ;
}

WikibaseEntity::WikibaseEntity ( WikibaseID id , std::shared_ptr<WikibaseAPI> api ) : id(id) {
	loadDataFromApi ( api ) ;
}

void WikibaseEntity::loadDataFromApi ( std::shared_ptr<WikibaseAPI> _api ) {
	api = _api ;
	if ( isDataLoaded() ) return ; // TODO Change to new API? Invalidate data? Load new data?
	if ( !isValidID(id) ) throw WikibaseException ( "WikibaseEntity::loadDataFromApi: Not a valid ID: " + id ) ;
	json result ;
	try {
		result = api->runQuery ( {
			{"action","wbgetentities"},
			{"ids",id}
		} ) ;
	} catch ( ... ) { // TODO throw on this?
		cerr << "Some problem loading " << id << endl ;
	}
	if ( result.count("entities") == 0 ) throw WikibaseException ( "No 'entities' in result for "+id , "WikibaseEntity::loadDataFromApi" ) ;
	if ( result["entities"].count(id) == 0 ) throw WikibaseException ( "No '"+id+"' in result entities" , "WikibaseEntity::loadDataFromApi" ) ;
	j = result["entities"][id] ;
}

bool WikibaseEntity::isValidID ( WikibaseID id ) {
	return std::regex_match ( id , std::regex("^[A-Z]\\d+$") ) ;
}

string WikibaseEntity::getEntityURL() {
	string url = api->getSiteInfo().at("wikibase-conceptbaseuri") ;
	url += id ;
	return url ;
}

string WikibaseEntity::getWebURL() {
	string title = j.at("title") ;

	string url = "https" ;
	url += api->getSiteInfo().at("server") ;
	url += api->getSiteInfo().at("articlepath") ;

	std::string::size_type pos = url.find("$1") ;
	if ( pos == std::string::npos ) throw WikibaseException ( "no $1 pattern: "+url , "WikibaseEntity::getWebURL" ) ;
	url.replace(pos, url.length(), title);
	return url ;
}

WikibaseEntityType WikibaseEntity::getType ( const WikibaseID &id ) {
	if ( id.empty() ) return WikibaseEntityType() ;
	return id[0] ;
}

bool WikibaseEntity::hasLabelInLanguage ( string language_code ) {
	if ( !isDataLoaded() ) throw WikibaseException ( "Data not loaded" , "WikibaseEntity::hasLabelInLanguage" ) ;
	if ( j.count("labels") == 0 ) return false ;
	return j["labels"].count(language_code) > 0 ;
}

string WikibaseEntity::getLabelInLanguage ( string language_code ) {
	if ( !hasLabelInLanguage(language_code) ) return "" ;
	return j["labels"][language_code].at("value") ;
}

bool WikibaseEntity::hasDescriptionInLanguage ( string language_code ) {
	if ( !isDataLoaded() ) throw WikibaseException ( "Data not loaded" , "WikibaseEntity::hasDescriptionInLanguage" ) ;
	if ( j.count("descriptions") == 0 ) return false ;
	return j["descriptions"].count(language_code) > 0 ;
}

string WikibaseEntity::getDescriptionInLanguage ( string language_code ) {
	if ( !hasDescriptionInLanguage(language_code) ) return "" ;
	return j["descriptions"][language_code].at("value") ;
}


bool WikibaseEntity::hasAliasesInLanguage ( string language_code ) {
	if ( !isDataLoaded() ) throw WikibaseException ( "Data not loaded" , "WikibaseEntity::hasAliasesInLanguage" ) ;
	if ( j.count("aliases") == 0 ) return false ;
	return j["aliases"].count(language_code) > 0 ;
}

vector <string> WikibaseEntity::getAliasesInLanguage ( string language_code ) {
	vector <string> ret ;
	if ( !hasAliasesInLanguage(language_code) ) return ret ;
	for ( auto& v:j["aliases"][language_code] ) {
		ret.push_back ( v.at("value") ) ;
	}
	return ret ;
}

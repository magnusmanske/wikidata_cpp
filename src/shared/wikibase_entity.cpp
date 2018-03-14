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
	if ( isDataLoaded() ) return ;
	if ( !isValidID(id) ) throw "WikibaseEntity::loadDataFromApi: Not a valid ID: " + id ;
	j = api->runQuery ( {
		{"action","wbgetentities"},
		{"ids",id}
	} ) ;
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
	if ( pos == std::string::npos ) throw "WikibaseEntity::getWebURL: no $1 pattern: "+url ;
	url.replace(pos, url.length(), title);
	return url ;
}

WikibaseEntityType WikibaseEntity::getType ( const WikibaseID &id ) {
	if ( id.empty() ) return WikibaseEntityType() ;
	return id[0] ;
}

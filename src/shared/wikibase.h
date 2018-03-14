#ifndef __wikibase_H__
#define __wikibase_H__

#include <string>
#include "../vendor/json.hpp"

using namespace std ;
using json = nlohmann::json;

typedef char WikibaseEntityType ;
typedef string WikibaseID ;
typedef vector <WikibaseID> WikibaseEntityList ;

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
	bool isInitialized() { return !id.empty() ; }
	void loadDataFromApi ( std::shared_ptr<WikibaseAPI> _api ) ;
	static bool isValidID ( WikibaseID id ) ;
	static WikibaseEntityType getType ( const WikibaseID &id ) ;
	WikibaseEntityType getType () { return getType(id) ; }
	string getEntityURL() ;
	string getWebURL() ;
	bool isDataLoaded() { return !j.empty() ; }

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
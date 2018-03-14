#include "wikibase.h"
#include <stdexcept>
#include <iostream>

const size_t ENTITY_BATCH_SIZE = 50 ;

bool WikibaseEntities::isEntityLoaded ( const WikibaseID &id ) {
	return entities.find(id) != entities.end() ;
}

string WikibaseEntities::joinEntityIDs ( const WikibaseEntityList &ids , const string &separator ) {
	string ret ;
	for ( auto &id:ids ) {
		if ( !ret.empty() ) ret += separator ;
		ret += id ;
	}
	return ret ;
}

WikibaseEntity WikibaseEntities::getEntity ( WikibaseID id , bool autoload ) {
	if ( isEntityLoaded(id) ) return entities[id] ;
	if ( !autoload ) return WikibaseEntity(id) ;
	loadEntities ( { id } ) ;
	return getEntity ( id , false ) ;
}

void WikibaseEntities::loadEntities ( const WikibaseEntityList &load_entities ) {
	typedef vector <WikibaseEntityList> WikibaseEntityBatches ;
	WikibaseEntityBatches batches = { {} } ;
	map <WikibaseID,bool> already_loading ;
	for ( auto &entity:load_entities ) {
		if ( entity.empty() ) continue ; // Tolerate empty strings
		if ( !WikibaseEntity::isValidID(entity) ) throw "WikibaseEntities::loadEntities: Invalid ID " + entity ;
		if ( isEntityLoaded(entity) ) continue ;
		if ( already_loading.find(entity) != already_loading.end() ) continue ;
		already_loading[entity] = true ;
		if ( batches[batches.size()-1].size() >= ENTITY_BATCH_SIZE ) batches.push_back ( WikibaseEntityList() ) ;
		batches[batches.size()-1].push_back ( entity ) ;
	}

	for ( auto &batch:batches ) {
		json j ;
		try {
			j = api->runQuery ( {
				{"action","wbgetentities"},
				{"ids",joinEntityIDs(batch,"|")}
			} ) ;
		} catch ( exception &e ) {
			cerr << e.what() << endl ;
			exit(1) ;
		}
		
		for (json::iterator it = j.at("entities").begin(); it != j.at("entities").end(); ++it) {
			entities[it.key()] = WikibaseEntity ( it.value() , api ) ;
		}

//		for ( auto &e:entities ) cout << e.second.getEntityURL() << endl ;
//		for ( auto &e:entities ) cout << e.second.getWebURL() << endl ;
	}
}
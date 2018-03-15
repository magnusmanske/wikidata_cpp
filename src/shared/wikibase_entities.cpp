#include "wikibase.h"
#include <stdexcept>
#include <iostream>
#include <thread>
#include <mutex>

std::mutex wikibase_entity_batches_mutex ;

class WikibaseEntityBatches {
public:
	WikibaseEntityBatches() {}
	void push_back ( WikibaseID id ) {
		if ( batches.size() == 0 || batches[batches.size()-1].size() >= batch_size ) {
			batches.push_back ( WikibaseEntityList() ) ;
			is_processed.push_back ( false ) ;
		}
		batches[batches.size()-1].push_back ( id ) ;
	}
	size_t getNextBatchToProcess () {
		std::lock_guard<std::mutex> lock(wikibase_entity_batches_mutex);
		size_t ret = batches.size() ; // Default: all done
		for ( size_t i = 0 ; i < is_processed.size() ; i++ ) {
			if ( is_processed[i] ) continue ;
			is_processed[i] = true ;
			ret = i ;
			break ;
		}
		return ret ;
	}
	string getJoinedBatch ( size_t batch_id , const string &separator ) {
		return joinEntityIDs ( batches[batch_id] , separator ) ;
	}
	auto begin () { return batches.begin() ; }
	auto end () { return batches.end() ; }
	auto size () { return batches.size() ; }

protected:
	string joinEntityIDs ( const WikibaseEntityList &ids , const string &separator ) {
		string ret ;
		for ( auto &id:ids ) {
			if ( !ret.empty() ) ret += separator ;
			ret += id ;
		}
		return ret ;
	}

	vector <WikibaseEntityList> batches ;
	vector <bool> is_processed ;
	uint32_t batch_size = 50 ;
} ;



bool WikibaseEntities::isEntityLoaded ( const WikibaseID &id ) {
	return entities.find(id) != entities.end() ;
}


WikibaseEntity WikibaseEntities::getEntity ( WikibaseID id , bool autoload ) {
	if ( isEntityLoaded(id) ) return entities[id] ;
	if ( !autoload ) return WikibaseEntity(id) ;
	loadEntities ( { id } ) ;
	return getEntity ( id , false ) ;
}

// 

std::mutex wikibaseEntities_loadEntities_mutex ;
void WikibaseEntities_loadEntities ( std::shared_ptr<WikibaseAPI> *api , WikibaseEntityBatches *batches , map <WikibaseID,WikibaseEntity> *entities ) {
	while ( 1 ) {

		size_t batch_id = batches->getNextBatchToProcess() ;
		if ( batch_id == batches->size() ) break ;

		json j ;
		try {
			j = (*api)->runQuery ( {
				{"action","wbgetentities"},
				{"ids",batches->getJoinedBatch(batch_id,"|")}
			} ) ;
		} catch ( exception &e ) {
			cerr << e.what() << endl ;
			exit(1) ;
		}
		
		std::lock_guard<std::mutex> lock(wikibaseEntities_loadEntities_mutex);
		for (json::iterator it = j.at("entities").begin(); it != j.at("entities").end(); ++it) {
			(*entities)[it.key()] = WikibaseEntity ( it.value() , (*api) ) ;
		}
	}
}

void WikibaseEntities::loadEntities ( const WikibaseEntityList &load_entities ) {
	WikibaseEntityBatches batches ;
	map <WikibaseID,bool> already_loading ;
	for ( auto &entity:load_entities ) {
		if ( entity.empty() ) continue ; // Tolerate empty strings
		if ( !WikibaseEntity::isValidID(entity) ) throw WikibaseException ( "WikibaseEntities::loadEntities: Invalid ID " + entity ) ;
		if ( isEntityLoaded(entity) ) continue ;
		if ( already_loading.find(entity) != already_loading.end() ) continue ;
		already_loading[entity] = true ;
		batches.push_back ( entity ) ;
	}

	vector <std::thread*> threads ;
	while ( threads.size() < max_threads && threads.size() < batches.size() ) {
		threads.push_back ( new std::thread ( &WikibaseEntities_loadEntities , &api , &batches , &entities ) ) ;
	}
	for ( auto t:threads ) t->join() ;
}
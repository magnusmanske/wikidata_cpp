#include "gtest/gtest.h"
#include "wikibase.h"

TEST(WikibaseAPI, site_info) {
	WikibaseAPI api ;
	json si ;
	try {
		si = api.getSiteInfo() ;
	} catch ( exception &e ) {
		cerr << "WikibaseAPI exception: " << e.what() << endl ;
	}
	ASSERT_FALSE(si.empty()) ;
	ASSERT_EQ(si.at("mainpage"),"Wikidata:Main Page") ;
}

TEST(WikibaseEntity, init_check) {
	WikibaseEntity wd_blank , wd_item ("Q12345") ;
	ASSERT_FALSE(wd_blank.isInitialized());
	ASSERT_TRUE(wd_item.isInitialized());
}

TEST(WikibaseEntity, load_check) {
	auto api = std::make_shared<WikibaseAPI>() ;
	WikibaseEntity wd_item1 ("Q12345") ;
	WikibaseEntity wd_item2 ("Q12345",api) ;
	ASSERT_FALSE(wd_item1.isDataLoaded());
	ASSERT_TRUE(wd_item2.isDataLoaded());
	wd_item1.loadDataFromApi(api) ;
	ASSERT_TRUE(wd_item1.isDataLoaded());
}

TEST(WikibaseEntity, id_check) {
	ASSERT_TRUE(WikibaseEntity::isValidID("Q12345"));
	ASSERT_FALSE(WikibaseEntity::isValidID("Foobar"));
	}

TEST(WikibaseEntities, load_check) {
	WikibaseEntities wel ;
	WikibaseEntityList entities = { "Q12345","P31","Q12345" } ;
	try {
		wel.loadEntities ( entities ) ;
	} catch ( exception &e ) {
		cerr << "!!" << e.what() << endl ;
	} catch ( string s ) {
		cerr << "!!!" << s << endl ;
	}
	ASSERT_EQ(wel.size(),2) ;
}

TEST(WikibaseEntities, get_check) {
	WikibaseID test_id ( "Q12345" ) ;
	WikibaseEntities wel ;
	ASSERT_FALSE(wel.isEntityLoaded(test_id));
	WikibaseEntity item1 = wel.getEntity ( test_id ) ;
	ASSERT_FALSE(item1.isDataLoaded());
	WikibaseEntity item2 = wel.getEntity ( test_id , true ) ;
	ASSERT_TRUE(item2.isDataLoaded());
}

int main ( int argc , char** argv ) {
	::testing::InitGoogleTest(&argc, argv);

	int returnValue =  RUN_ALL_TESTS();
	return returnValue ;
}
#include "gtest/gtest.h"
#include <algorithm>
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

TEST(WikibaseEntity, detail_check) {
	auto api = std::make_shared<WikibaseAPI>() ;
	WikibaseEntity count_von_count ("Q12345",api) ;
	ASSERT_TRUE(count_von_count.hasLabelInLanguage("en")) ;
	ASSERT_FALSE(count_von_count.hasLabelInLanguage("this-is-not-a-language-code")) ;
	ASSERT_EQ(count_von_count.getLabelInLanguage("en"),"Count von Count") ;

	ASSERT_TRUE(count_von_count.hasDescriptionInLanguage("en")) ;
	ASSERT_FALSE(count_von_count.hasDescriptionInLanguage("this-is-not-a-language-code")) ;
	ASSERT_EQ(count_von_count.getDescriptionInLanguage("en"),"character on Sesame Street") ; // This might change on Wikidata

	ASSERT_TRUE(count_von_count.hasAliasesInLanguage("en")) ;
	ASSERT_FALSE(count_von_count.hasAliasesInLanguage("this-is-not-a-language-code")) ;
	ASSERT_GE(count_von_count.getAliasesInLanguage("en").size(),1) ;

	ASSERT_TRUE(count_von_count.hasSitelinkToWiki("enwiki")) ;
	ASSERT_FALSE(count_von_count.hasSitelinkToWiki("this-is-not-a-wiki-code")) ;
	ASSERT_EQ(count_von_count.getSitelinkToWiki("enwiki"),"Count von Count") ;

	ASSERT_EQ(count_von_count.getEntityType(),"item") ;
	ASSERT_EQ(count_von_count.getPageID(),13925) ;
	ASSERT_EQ(count_von_count.getPageTitle(),"Q12345") ;

	// Claims
	ASSERT_TRUE(count_von_count.hasClaimsForProperty("P31")) ;
	ASSERT_FALSE(count_von_count.hasClaimsForProperty("P007")) ;

	auto res1 = count_von_count.getTargetItemsForProperty("P31") ;
	ASSERT_NE(std::find(res1.begin(),res1.end(),"Q30061417"),res1.end()) ; // This might change on Wikidata

	auto res2 = count_von_count.getStringsForProperty("P345") ;
	ASSERT_NE(std::find(res2.begin(),res2.end(),"ch0000709"),res2.end()) ; // This might change on Wikidata

	// Badges
	WikibaseEntity angkor_wat ("Q43473",api) ;
	ASSERT_TRUE(angkor_wat.hasBadgesInWiki("enwiki")) ;
	ASSERT_FALSE(angkor_wat.hasBadgesInWiki("elwiki")) ; // This might change on Wikidata
	ASSERT_FALSE(angkor_wat.hasBadgesInWiki("this-is-not-a-wiki-code")) ;
	ASSERT_GE(angkor_wat.getBadgesInWiki("enwiki").size(),1) ;
}

TEST(WikibaseEntities, load_check) {
	WikibaseEntities wel ;
	WikibaseEntityList entities = { "Q12345","P31","Q12345" } ;
	try {
		wel.loadEntities ( entities ) ;
	} catch ( WikibaseException &e ) {
		cerr << "!!" << e.what() << endl ;
	} catch ( string s ) {
		cerr << "!!!" << s << endl ;
	} catch ( ... ) {
		cerr << "Something is wrong" << endl ;
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

#include <gtest/gtest.h>

#include "private/default_configuration.hpp"
#include "entrydata.hpp"

TEST(DefaultConfigurationShould, returnDefaultEntries)
{
    DefaultConfiguration configuration;
    
    const std::vector<EntryData>& entries = configuration.getEntries();
    
    ASSERT_EQ(false, entries.empty());
}


TEST(DefaultConfigurationShould, throwWhenEntryAdded)
{
    DefaultConfiguration configuration;
    
    ASSERT_THROW
    (
        configuration.addEntry(EntryData("Test", "Entry")),
        std::logic_error
    );
}

#include "software/LocalName.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(LocalNameTest, DerivesShortAndCanonicalNamesFromEligibleSoftwareName)
{
  const auto name = softadastra::LocalName::from_software_name("phone-test");

  ASSERT_TRUE(name.has_value());
  EXPECT_EQ(name->label(), "phone-test");
  EXPECT_EQ(name->short_name(), "phone-test");
  EXPECT_EQ(name->canonical_name(), "phone-test.softadastra.home.arpa");
}

TEST(LocalNameTest, AcceptsEligibleDnsLabels)
{
  EXPECT_TRUE(softadastra::LocalName::from_software_name("phone-test").has_value());
  EXPECT_TRUE(softadastra::LocalName::from_software_name("api").has_value());
  EXPECT_TRUE(softadastra::LocalName::from_software_name("a1").has_value());
  EXPECT_TRUE(softadastra::LocalName::from_software_name(std::string(63, 'a')).has_value());
}

TEST(LocalNameTest, RejectsIneligibleSoftwareNamesWithoutNormalization)
{
  const std::vector<std::string> names{
      "", "My API", "API", "api_test", "foo.bar", "\xC3\xA9"
                                                  "cole",
      "-api",
      "api-", std::string(64, 'a')};
  for (const std::string &name : names)
  {
    EXPECT_FALSE(softadastra::LocalName::from_software_name(name).has_value())
        << name;
  }
}

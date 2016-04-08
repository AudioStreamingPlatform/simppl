#include <gtest/gtest.h>

#include "simppl/stub.h"
#include "simppl/skeleton.h"
#include "simppl/dispatcher.h"
#include "simppl/interface.h"


using namespace std::placeholders;


namespace test
{
   namespace variant
   {
      // FIXME make complete test with struct
      struct Hello
      {
         typedef simppl::dbus::make_serializer<int,std::string>::type serializer_type;

         int i;
         double d;
      };


      INTERFACE(VServer)
      {
         Request<> getData;
         Response<std::map<std::string, simppl::Variant<int,double,std::string>>> rGetData;

         VServer()
          : INIT(getData)
          , INIT(rGetData)
         {
            getData >> rGetData;
         }
      };
   }
}


namespace {

   int constructs = 0;
   int destructs = 0;

   struct TestHelper
   {
      TestHelper()
      {
         ++constructs;
      }

      TestHelper(const TestHelper&)
      {
         ++constructs;
      }

      ~TestHelper()
      {
         ++destructs;
      }
   };


   struct Client : simppl::dbus::Stub<test::variant::VServer>
   {
      Client()
       : simppl::dbus::Stub<test::variant::VServer>("role")
      {
         connected >> std::bind(&Client::handleConnected, this, _1);
         rGetData >> std::bind(&Client::handleGetData, this, _1, _2);
      }

      void handleConnected(simppl::dbus::ConnectionState s)
      {
         getData();
      }

      void handleGetData(simppl::dbus::CallState state, const std::map<std::string, simppl::Variant<int,double,std::string>>& mapping)
      {
         EXPECT_EQ(3, mapping.size());

         auto hello = mapping.find("Hello");
         auto world = mapping.find("World");
         auto toll = mapping.find("Tolle");

         EXPECT_NE(mapping.end(), hello);
         EXPECT_NE(mapping.end(), world);
         EXPECT_NE(mapping.end(), toll);

         EXPECT_EQ(42, *hello->second.get<int>());
         EXPECT_EQ(4711, *world->second.get<int>());
         EXPECT_EQ(std::string("Show"), *toll->second.get<std::string>());

         disp().stop();
      }
   };

   struct Server : simppl::dbus::Skeleton<test::variant::VServer>
   {
      Server()
       : simppl::dbus::Skeleton<test::variant::VServer>("role")
      {
         getData >> std::bind(&Server::handleRequest, this);
      }

      void handleRequest()
      {
         std::map<std::string, simppl::Variant<int,double,std::string>> mapping;
         mapping["Hello"] = 42;
         mapping["World"] = 4711;
         mapping["Tolle"] = std::string("Show");

         respondWith(rGetData(mapping));
      }
   };
}


TEST(Variant, basic)
{
   simppl::Variant<int, double, std::string, TestHelper> v;

   v = 42;
   EXPECT_EQ(42, *v.get<int>());

   v = std::string("Hallo Welt");
   EXPECT_EQ(std::string("Hallo Welt"), *v.get<std::string>());

   v = TestHelper();

   v = 43;
   EXPECT_EQ(43, *v.get<int>());

   EXPECT_EQ(2, constructs);
   EXPECT_EQ(2, destructs);
}


TEST(Variant, map)
{
   simppl::Variant<std::map<int, std::string> > v;

   std::map<int, std::string> m {
      { 1, "Hallo" },
      { 2, "Welt" }
   };

   v = m;
   EXPECT_EQ(2, (v.get<std::map<int, std::string>>()->size()));

   int i=0;
   for(auto& e : *v.get<std::map<int, std::string>>())
   {
      if (i == 0)
      {
         EXPECT_EQ(1, e.first);
         EXPECT_EQ(std::string("Hallo"), e.second);
      }
      else if (i == 1)
      {
         EXPECT_EQ(2, e.first);
         EXPECT_EQ(std::string("Welt"), e.second);
      }

      ++i;
   }

   EXPECT_EQ(2, i);
}


TEST(Variant, method)
{
   simppl::dbus::Dispatcher d("bus:session");
   Client c;
   Server s;

   d.addClient(c);
   d.addServer(s);

   d.run();
}

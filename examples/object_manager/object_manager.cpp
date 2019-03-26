#include "simppl/skeleton.h"
#include "simppl/interface.h"
#include "simppl/map.h"
#include "simppl/vector.h"
#include "simppl/struct.h"
#include "simppl/string.h"
#include "simppl/variant.h"


namespace test
{


INTERFACE(Person)
{
    Property<std::string> name;

    Person()
      : INIT(name)
    {}
};


}


struct Person : simppl::dbus::Skeleton<test::Person>
{
    Person(simppl::dbus::Dispatcher& d, const char* key, const char* name)
      : simppl::dbus::Skeleton<test::Person>(d, "example.ObjectManager", std::string("/") + key)
    {
        this->name = name;
    }
};


struct People : simppl::dbus::Skeleton<>
{
    People(simppl::dbus::Dispatcher& d)
      : simppl::dbus::Skeleton<>(d, "example.ObjectManager", "/")
    {}
};


int main()
{
    simppl::dbus::Dispatcher d("bus:session");
    People p(d);
    Person a(d, "a", "Person A");
    Person b(d, "b", "Person B");

    p.add_child(a);
    p.add_child(b);

    d.run();
    return 0;
}

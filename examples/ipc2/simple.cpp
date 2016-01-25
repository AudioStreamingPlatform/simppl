#include "simppl/stub.h"
#include "simppl/skeleton.h"
#include "simppl/interface.h"
#include "simppl/dispatcher.h"
#include "simppl/blocking.h"


using namespace std::placeholders;

namespace spl = simppl::ipc;


namespace test
{


struct A
{
   typedef spl::make_serializer<int, int>::type serializer_type;

   int i;
   int j;
};


INTERFACE(Simple)
{
   Request<int> echo;
   Response<int> rEcho;

   Request<A, int> echoA;
   Request<std::vector<int>, int> echoVi;
   Request<std::vector<A>, int> echoVa;
   Request<std::tuple<int, std::string, int>> echoTup;

   Signal<double> sigUsr;

   Attribute<int> attInt;

   Simple()
    : INIT_REQUEST(echo)
    , INIT_RESPONSE(rEcho)
    , INIT_REQUEST(echoA)
    , INIT_REQUEST(echoVi)
    , INIT_REQUEST(echoVa)
    , INIT_REQUEST(echoTup)
    , INIT_SIGNAL(sigUsr)
    , INIT_ATTRIBUTE(attInt)
   {
      echo >> rEcho;
   }
};


INTERFACE(Simple2)
{
   Request<int> echo;
   Response<int> rEcho;

   Signal<double> sigUsr;

   Simple2()
    : INIT_REQUEST(echo)
    , INIT_SIGNAL(sigUsr)
    , INIT_RESPONSE(rEcho)
   {
      echo >> rEcho;
   }
};

}   // namespace


// ---------------------------------------------------------------------


void handle_connected(spl::ConnectionState s)
{
    std::cout << "Connectionstate changed: " << ((int)s==0?"Connected":"Disconnected") << std::endl;
}


void sig_callback(double d)
{
    std::cout << "Having signal " << d << std::endl;
}

spl::Stub<test::Simple>* psst = 0;

void att_callback(int i)
{
    static int count = 0;
   std::cout << "Having attribute change " << i << std::endl;
   if (++count == 1)
      psst->echo(1111);
}


void echo_callback(const spl::CallState&, int i)
{
   std::cout << "Reponse from server: " << i << std::endl;
}


int client()
{
   spl::Stub<test::Simple> sst("my_simple");
   psst = &sst;

   sst.connected >> &handle_connected;

   spl::Dispatcher disp;
   disp.addClient(sst);

   int j = 0;
   sst.echo(55) >> j;
   std::cout << "j=" << j << std::endl; 

   test::A a = { 3, 4 };
   sst.echoA(a, 77);

   std::vector<int> vi({ 42, 43 });
   sst.echoVi(vi, 88);

   std::vector<test::A> va;
   va.push_back(a);
   a.i = 7;
   a.j = 8;
   va.push_back(a);
   sst.echoVa(va, 99);

   std::tuple<int, std::string, int> tup(42, "Hallo Welt", 666);
   sst.echoTup(tup);

   sst.rEcho >> echo_callback;

   sst.sigUsr.attach() >> sig_callback;
   sst.echo(42);

   sst.attInt.attach() >> att_callback;

   return disp.run();
}


// ---------------------------------------------------------------------


struct SimpleServer2 : spl::Skeleton<test::Simple2>
{
    SimpleServer2()
     : spl::Skeleton<test::Simple2>("my_simple2")
    {
        echo >> std::bind(&SimpleServer2::handleEcho, this, _1);
    }

    void handleEcho(int i)
    {
        std::cout << "Client saying '" << i << "'" << std::endl;
        sigUsr.emit(3.1415);

        respondWith(rEcho(42));
    }
};

void handleEchoA(test::A a, int i)
{
   std::cout << "Having " << a.i << " " << a.j << " and " << i << std::endl;
}


void handleEchoVi(const std::vector<int>& vi, int i)
{
   std::cout << "Having " << vi.size() << ": ";
   std::for_each(vi.begin(), vi.end(), []( int i){ std::cout << i << ", "; });
   std::cout << "and " << i << std::endl;
}


void handleEchoVa(const std::vector<test::A>& va, int i)
{
   std::cout << "Having " << va.size() << ": ";
   std::for_each(va.begin(), va.end(), [](const test::A& a){ std::cout << "[" << a.i << "," << a.j << "], "; });
   std::cout << "and " << i << std::endl;
}


void handleEchoTup(const std::tuple<int, std::string, int>& t)
{
   std::cout << "Having " << std::get<0>(t) << " -> " << std::get<1>(t) << " " << std::get<2>(t) << std::endl;
}


struct SimpleServer : spl::Skeleton<test::Simple>
{
    SimpleServer()
     : spl::Skeleton<test::Simple>("my_simple")
    {
        attInt = 4711;
        echo >> std::bind(&SimpleServer::handleEcho, this, _1);
        echoA >> handleEchoA;
        echoVi >> handleEchoVi;
        echoVa >> handleEchoVa;
        echoTup >> handleEchoTup;
    }

    void handleEcho(int i)
    {
        static int count = 0;

        ++count;

        std::cout << "Client saying '" << i << "'" << std::endl;
        sigUsr.emit(3.1415);

        respondWith(rEcho(42));

        if (count == 2)
            attInt = 5544;
    }
};


int server()
{
   SimpleServer serv;

   spl::Dispatcher disp;
   disp.addServer(serv);

   return disp.run();
}


void dummy()
{
   DBusMessage* msg;

    std::tuple<int, double> tup;
    std::vector<int> vi;
    test::A a;
    std::vector<test::A> va;

    spl::detail::Serializer s(msg);

    s << a;
    s << tup;
    s << vi;
    s << va;

    spl::detail::Deserializer ds(msg);
    ds >> a;
    ds >> tup;
    ds >> vi;
    ds >> va;
}


int main(int argc, char** argv)
{
   // never call, will core dump, just a compile test!
   if (argc > 10)
      dummy();

    return argc > 1 ? server() : client();
}
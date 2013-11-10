
#include "Test.h"
#include "Mona/SocketAddress.h"

using namespace std;
using namespace Mona;


ADD_TEST(SocketAddress, Test) {
	
	SocketAddress sa;
	Exception ex;

	CHECK(sa.host().isWildcard());
	CHECK(sa.port() == 0);

	sa.set(ex,"192.168.1.100", 100);
	CHECK(sa.host().toString() == "192.168.1.100");
	CHECK(sa.port() == 100);

	sa.set(ex, "192.168.1.100", "100");
	CHECK(sa.host().toString() == "192.168.1.100");
	CHECK(sa.port() == 100);


	sa.set(ex, "192.168.1.100", "ftp");
	CHECK(sa.host().toString() == "192.168.1.100");
	CHECK(sa.port() == 21);

	CHECK(!ex);

	sa.set(ex,"192.168.1.100", "f00bar");
	CHECK(ex);

	ex.set(Exception::NIL, "");

	sa.set(ex,"www.appinf.com", 80);
	CHECK(sa.host().toString() == "50.57.108.29");
	CHECK(sa.port() == 80);

	CHECK(!ex);

	sa.set(ex,"192.168.2.260", 80);

	CHECK(ex);
	ex.set(Exception::NIL, "");

	sa.set(ex, "192.168.2.120", "80000");
	
	CHECK(ex);
	ex.set(Exception::NIL, "");

	sa.set(ex,"192.168.2.120:88");
	CHECK(sa.host().toString() == "192.168.2.120");
	CHECK(sa.port() == 88);

	sa.set(ex, "[192.168.2.120]:88");
	CHECK(sa.host().toString() == "192.168.2.120");
	CHECK(sa.port() == 88);

	CHECK(!ex);

	sa.set(ex, "[192.168.2.260]");
	CHECK(ex);
	ex.set(Exception::NIL, "");

	sa.set(ex,"[192.168.2.260:88");
	CHECK(ex);
	ex.set(Exception::NIL, "");
	
	sa.set(ex,"192.168.1.100", 100);
	SocketAddress sa2;
	sa2.set(ex,"192.168.1.100:100");
	CHECK(sa == sa2);

	sa.set(ex,"192.168.1.101", "99");
	CHECK(sa2 < sa);

	sa2.set(ex, "192.168.1.101", "102");
	CHECK(sa < sa2);

	CHECK(!ex);
}


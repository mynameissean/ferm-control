/*#include "..\..\Arduino\Unity\src\unity.h"
#include "..\..\Arduino\Unity\extras\fixture\src\unity_fixture.h"
#include "TemperatureSensor.h"
#include "ID.h"

TEST_GROUP(TemperatureSensor);
TEST_SETUP(TemperatureSensor)
{

}

TEST_TEAR_DOWN(TemperatureSensor)
{
}

TEST(TemperatureSensor, DoNothing)
{
	byte* address = new byte[8];
	ID* id = new ID("NULL", 5, 0);
	TemperatureSensor* sensor = new TemperatureSensor(address, 1.5, 60, id);
	TEST_ASSERT_EQUAL(1, sensor->foo());
}

TEST_GROUP_RUNNER(mod1)
{
	RUN_TEST_CASE(mod1, DoNothing);
}

static void RunAllTests()
{
	RUN_TEST_GROUP(mod1);
}

int main(int argc, const char *argv[])
{
	return UnityMain(argc, argv, RunAllTests);
}*/
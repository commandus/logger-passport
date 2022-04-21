#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>
#include "logger-plume-collection.h"

static const std::string Plumes = "N6-3\n"
    "282AA47804000061\t1,01456\t-0,18723\n"
    "# comment at the beginning\n"
    "287027790400008A\t1,01348\t-0,12368\n"
    "287C257904000074\t1,0117\t-0,14454   # comment at the end\n"
    "2817B979040000D9\t1,0142\t-0,06039\n"
    "# multiline comment 1\n"
    "# multiline comment 2\n"
    "# multiline comment 3\n"
    "28F329790400009B\t1,01025\t-0,06014\n"
    "289FBC78040000CF\t1,0135\t-0,18703\n"
    "2855A7790400001C\t1,0142\t-0,06039\n"
    "28D43A790400009A\t1,01171\t-0,18669\n"
    "28A0E579040000EF\t1,01168\t-0,06023\n"
    "28067C7904000045\t1,01132\t0,003\n"
    "283642790400008E\t1,00957\t-0,12319\n"
    "2857C17804000070\t1,01205\t-0,08134\n"
    "282A2579040000E8\t1,01312\t-0,06032\n"
    "2878BF780400002D\t1,01384\t-0,10261\n"
    "28124579040000B5\t1,01601\t-0,25099\n"
    "28C1B37904000012\t1,01528\t-0,12391\n"
    "N6-2\n"
    "2809B618040000E1\t1,01280\t\t-0,14103\t\t\t\n"
    "28B3A71804000098\t1,01173\t\t-0,05657\t\t\n"
    "28858F1804000001\t1,01496\t\t-0,16249\t\t\t\n"
    "280599180400000B\t1,01280\t\t-0,11993\t\t\n"
    "28D87B18040000DC\t1,01139\t\t-0,05654\t\t\n"
    "2878B018040000D2\t1,01421\t\t-0,12011\t\t\n"
    "2894981804000040\t1,01352\t\t-0,09891\t\t\n"
    "2837941804000064\t1,01639\t\t-0,20508\t\t\n"
    "281AAF180400006C\t1,01315\t\t-0,01444\t\t\n"
    "2836E8780400008F\t1,01269\t\t-0,06896\t\t\n"
    "28FBD0790400006F\t1,01306\t\t-0,1323\t\t\t\n"
    "28C03E7904000002\t1,01382\t\t-0,21687\t\t\n"
    "28578F79040000B4\t1,01485\t\t-0,13252\t\t\n"
    "28943C7904000073\t1,00985\t\t-0,1319\t\t\t\n"
    "28F6DE7804000008\t1,01592\t\t-0,19615\t\t\n"
    "282E922F0200002A\t0,03362\t\t1,03467\t\t0,00075\t\n"
    "\t\t\t-0,02038  \t1,00840\t\t-0,00131\t\n"
    "\t\t\t0,02143\t\t0,98556\t\t0,00048\t\n"
    "283C902F0200009C\t0,05652\t\t1,01755\t\t0,00005\n"
    "\t\t\t0,04129\t\t1,01006\t\t-0,00056\t\n"
    "\t\t\t0,04502\t\t1,00512\t\t-0,00009\t\n"
    "2825B72E02000077\t0,08834\t\t0,99186\t\t-0,00067\t\n"
    "\t\t\t0,09958\t\t1,00262\t\t0,00032\n"
    "\t\t\t0,08762\t\t1,00868\t\t-0,00017\t\n"
    "2817A02F02000056\t-0,06935\t                1,00658\t\t-0,00037\t\n"
    "\t\t\t-0,05635\t                1,01099\t\t-0,00013\t\n"
    "\t\t\t-0,07709   \t1,01626\t\t-0,00038\t\n"
    "282AAB2F02000081\t-0,07790\t                1,01487\t\t0,00008\t\n"
    "\t\t\t-0,06815\t\t1,01403\t\t-0,00012\t\n"
    "\t\t\t-0,06032\t\t1,01632\t\t-0,00044\t\n"
    "N6-1\n"
    "28BC83180400005B\t1,01028\t\t0,02772\t        \t\t\n"
    "2874B11804000062\t1,01353    \t-0,07779\t\t\t\t\n"
    "282B981804000063\t1,01532\t\t-0,18370\t\t\t\n"
    "28D8B318040000C0\t1,01282\t\t-0,14103\t\t\t\n"
    "28DD8718040000B0\t1,01101\t\t-0,01440\t\t\t\t\n"
    "2813AD1804000079\t1,00995\t\t-0,05646\t\t\t\n"
    "2868B61804000015\t1,01100\t\t-0,05652\t\t\t\n"
    "28CB7F18040000C1\t1,01316\t\t-0,11998\t\t\t\t\n"
    "28C08018040000F0\t1,01172\t\t-0,05657\t\t\t\t\n"
    "28EC9918040000B4\t1,01895\t\t-0,24806\t\t\t\t\n"
    "2841B81804000097\t1,01495\t\t-0,18363\t\t\t\t\n"
    "28408D18040000F6\t1,01352\t\t-0,05668\t\t\t\n"
    "288EA31804000020\t1,01315\t\t-0,07776\t\t\t\n"
    "28FF8918040000CA\t1,01388\t\t-0,09894\t\t\t\n"
    "28D9951804000093\t1,01138\t\t-0,09868\t\t\t\n"
    "28098B2F02000096\t-0,04631\t\t1,00489\t\t-0,00013\t\n"
    "\t               \t\t -0,04846\t\t1,00386\t\t-0,00018\t\n"
    "\t\t\t-0,03425\t\t0,99884\t\t0,00016\t\t\n"
    "2899CA2E0200008C\t-0,03703\t\t1,00986\t\t0,00000\n"
    "\t\t\t-0,02906\t\t1,01433\t\t0,00032\t\n"
    "\t\t\t-0,05726\t\t1,02679\t\t-0,00063\t\n"
    "28B19F2F02000053\t0,15545\t\t1,00265\t\t-0,00011\t\n"
    "\t\t\t0,15283\t\t1,00253\t\t-0,00004\t\n"
    "\t\t\t0,17191\t\t0,99911\t\t0,00005\n"
    "28F0A62F02000066\t0,00530\t\t1,02273\t\t0,00022\t\n"
    "\t\t\t-0,02963\t\t1,01192\t\t-0,00042\t\n"
    "\t\t\t-0,01133\t\t1,00424\t\t0,00007\t\n"
    "28C4902F0200004F\t-0,14482\t\t0,99694\t\t-0,00051\t\n"
    "\t\t\t-0,11780\t\t1,00866\t\t0,00034\n"
    "\t\t\t-0,14156\t\t1,01919\t\t-0,00041\t\n"
    "NEND";

static void testParseText()
{
    LoggerPlumeCollection *c = new LoggerPlumeMemory();
    c->loadString(time(nullptr), Plumes);
    std::cout << "Plumes: " << c->count() << std::endl;
    assert(c->count() == 3);
    std::cout << "Plume N6-1 sensor count: " << c->get(1, 6)->sensors.size() << std::endl;
    std::cout << "Plume N6-2 sensor count: " << c->get(2, 6)->sensors.size() << std::endl;
    std::cout << "Plume N6-3 sensor count: " << c->get(3, 6)->sensors.size() << std::endl;

    assert(c->get(1, 6)->sensors.size() == 20);
    assert(c->get(2, 6)->sensors.size() == 20);
    assert(c->get(3, 6)->sensors.size() == 16);

    std::cout << "Plume N6-3 sensor 0: " << c->get(3, 6)->sensors[0].toJsonString() << std::endl;
    std::cout << "Plume N6-3 sensor 1: " << c->get(3, 6)->sensors[1].toJsonString() << std::endl;

    assert(fabs(c->get(3, 6)->sensors[0].coefficients[0][0] - 1.01456) < 0.00001);
    assert(fabs(c->get(3, 6)->sensors[0].coefficients[0][1] - (-0.18723)) < 0.00001);
    assert(c->get(3, 6)->sensors[0].mac == 0x282AA47804000061);
    delete c;
}

static void testParseJson()
{
    LoggerPlumeCollection *c = new LoggerPlumeMemory();
    c->loadString(time(nullptr), Plumes, false);
    std::string json = c->toJsonString();
    LoggerPlumeCollection *c2 = new LoggerPlumeMemory();
    c2->loadString(time(nullptr), json, true);
    std::string json2 = c2->toJsonString();

    assert(json == json2);

    delete c;
    delete c2;
}

static void testSaveText()
{
    LoggerPlumeCollection *c = new LoggerPlumeMemory();
    c->loadString(time(nullptr), Plumes, false);

    std::string text = c->toString();
    std::cout << std::endl;
    std::cout << text << std::endl;
    std::cout << std::endl;

    delete c;
}

static void testTemperature()
{
    LoggerPlumeCollection *c = new LoggerPlumeMemory();
    c->startModification();
    c->loadString(time(nullptr), Plumes, false);
    c->finishModification();

    std::cout << std::endl;
    std::cout << "MAC\t\tT by order\tT by MAC\tdiff"<<std::endl;
        
    for (int t = -30; t < -29; t++) {
        int cnt = 0;
        for (auto it(c->get(1, 6)->sensors.begin()); it != c->get(1, 6)->sensors.end(); it++) {
            double t1 = c->calc(1, 6, cnt, (double) t);
            double t2 = c->calc(it->mac.mac.u, (double) t);
            std::cout << it->mac.toString() << std::fixed << std::setprecision(5) << " "
                << t1 << " " << t2
                << " diff: " << fabs(t2 - t1)
                << std::endl;
            cnt++;    
        }
    }
    std::cout << std::endl;
    delete c;
}

int main(int argc, char **argv)
{
    testParseText();
    testParseJson();
    testSaveText();
    testTemperature();
}

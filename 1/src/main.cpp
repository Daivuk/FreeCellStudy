#include <imgui/imgui.h>
#include <onut/Renderer.h>
#include <onut/Settings.h>

void initSettings()
{
    oSettings->setGameName("Advent of Code 2");
    oSettings->setIsResizableWindow(true);
    oSettings->setResolution({1280, 720});
    oSettings->setShowFPS(true);
    oSettings->setShowOnScreenLog(true);
}

void shutdown()
{
}

void update()
{
}

void render()
{
    oRenderer->clear();
}

void postRender()
{
}

#include <chrono>
#include <future>
#include <vector>
#include <onut/Async.h>
#include <onut/Dispatcher.h>

using namespace std;
using namespace chrono_literals;

const vector<int> puzzle_data = {
    2000, 50, 1984, 1600, 1736, 1572, 2010, 1559, 1999, 1764, 1808, 1745, 1343, 1495, 1860, 1977, 1981, 1640, 1966, 1961, 1978, 1719, 1930, 535, 1804, 1535, 1507, 1284, 1618, 1991, 1589, 1593, 1960, 1953, 1963, 1697, 1741, 1823, 1932, 1789, 1822, 1972, 1570, 1651, 1800, 1514, 726, 1567, 72, 1987, 1791, 1842, 1020, 1541, 1383, 1505, 2009, 1925, 13, 1973, 1599, 1632, 1905, 1626, 1554, 1913, 1890, 1583, 1513, 1828, 187, 1616, 1508, 1524, 1613, 1648, 32, 1612, 1992, 1671, 1955, 1943, 1936, 1870, 1629, 1493, 1770, 1699, 1990, 1658, 1592, 1596, 1888, 1540, 239, 1677, 1602, 1877, 1481, 2004, 1985, 1829, 1980, 2008, 1964, 897, 1843, 1750, 1969, 1790, 1989, 1606, 1484, 1983, 1986, 1501, 1511, 1543, 1869, 1051, 1810, 1716, 1633, 1850, 1500, 1120, 1849, 1941, 1403, 1515, 1915, 1862, 2002, 1952, 1893, 1494, 1610, 1797, 1908, 1534, 1979, 2006, 1971, 1993, 1432, 1547, 1488, 1642, 1982, 1666, 1856, 1889, 1691, 1976, 1962, 2005, 1611, 1665, 1816, 1880, 1896, 1552, 1809, 1844, 1553, 1841, 1785, 1968, 1491, 1498, 1995, 1748, 1533, 1988, 2001, 1917, 1788, 1537, 1659, 1574, 1724, 1997, 923, 1476, 1763, 1817, 1998, 1848, 1974, 1830, 1672, 1861, 1652, 1551, 1363, 1645, 1996, 1965, 1967, 1778
};

int processing_count = 0;
int result1 = -1;
int result2 = -1;

int calculateExpenseReport1()
{
    for (auto it = puzzle_data.begin(); it != puzzle_data.end(); it++)
    {
        for (auto it2 = puzzle_data.begin(); it2 != puzzle_data.end(); it2++)
        {
            if (it == it2) continue;

            auto a = *it;
            auto b = *it2;

            if (a + b == 2020)
            {
                return a * b;
            }
        }
    }

    return -1;
}

int calculateExpenseReport2()
{
    for (auto it = puzzle_data.begin(); it != puzzle_data.end(); it++)
    {
        for (auto it2 = puzzle_data.begin(); it2 != puzzle_data.end(); it2++)
        {
            if (it == it2) continue;
            for (auto it3 = puzzle_data.begin(); it3 != puzzle_data.end(); it3++)
            {
                if (it == it3) continue;
                if (it2 == it3) continue;

                auto a = *it;
                auto b = *it2;
                auto c = *it3;

                if (a + b + c == 2020)
                {
                    return a * b * c;
                }
            }
        }
    }

    return -1;
}

void init()
{
    processing_count = 2;
    
    thread thread1([]()
    {
        auto result = calculateExpenseReport1();
        OSync([=]()
        {
            result1 = result;
            processing_count--;
        });
    });
    thread1.detach();

    thread thread2([]()
    {
        auto result = calculateExpenseReport2();
        OSync([=]()
        {
            result2 = result;
            processing_count--;
        });
    });
    thread2.detach();
}

void renderUI()
{
    ImGui::Begin("Result");

    if (processing_count > 0)
    {
        const char spinner[] = {'/', '-', '\\', '|'};
        static int spinner_anim = 0;
        spinner_anim++;
        ImGui::LabelText("Processing", "%c", spinner[(spinner_anim / 4) % 4]);
    }

    ImGui::InputInt("Result 1", &result1);
    ImGui::InputInt("Result 2", &result2);

    ImGui::End();
}

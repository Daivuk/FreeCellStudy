#pragma once

#include <memory>

using namespace std;

class Board;
class Resources;

using BoardRef = shared_ptr<Board>;
using ResourcesRef = shared_ptr<Resources>;

extern BoardRef g_board;
extern ResourcesRef g_resources;

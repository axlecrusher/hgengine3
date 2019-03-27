#pragma once

/* This file should only be included once in the main cpp file of the program.*/

namespace ENGINE
{

class GlobalInitalizer
{
public:
	GlobalInitalizer();
private:
};

}

static ENGINE::GlobalInitalizer _globalEngineInit;
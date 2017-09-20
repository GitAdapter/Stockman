#include <Misc\GUI\MenuMachine.h>
#include <iostream>
#include <Misc\FileLoader.h>
using namespace Logic;

MenuMachine::MenuMachine()
{
	pressed = false;
	currentActiveMenu = nullptr;
	currentActiveState = gameStateMenuMain;

}


MenuMachine::~MenuMachine()
{
	for (const auto& a : m_menuStates)
	{
		delete a.second;
	}
}

void Logic::MenuMachine::initialize(GameState state)
{
	//Structure to define everything one Menu needs
	struct Menu
	{
		int theState;
		std::string background;
		std::vector<MenuState::ButtonStruct> buttons;
	};

	//Gather all the functions in a map for future allocation
	std::map<std::string, std::function<void(void)>> functions;
	functions["buttonClick0"] = std::bind(&MenuMachine::buttonClick0, this);
	functions["buttonClick1"] = std::bind(&MenuMachine::buttonClick1, this);
	functions["buttonClick2"] = std::bind(&MenuMachine::buttonClick2, this);

	//Load the lw file information
	std::vector<FileLoader::LoadedStruct> buttonFile;
	FileLoader::singleton().loadStructsFromFile(buttonFile, "Button");
	std::vector<FileLoader::LoadedStruct> menuFile;
	FileLoader::singleton().loadStructsFromFile(menuFile, "Menu");

	//Gather all the buttons in a map for future allocation
	std::map<std::string, MenuState::ButtonStruct> allButtons;

	for (auto const& button : buttonFile)
	{
		//If it is a button add it into its map
		if (button.strings.find("buttonName") != button.strings.end())
		{
			std::string testinging = button.strings.at("function"); //crap used until the buggy file handler is fixed
			testinging = testinging.substr(0, testinging.size() - 1); //crap used until the buggy file handler is fixed
			allButtons[button.strings.at("buttonName")] = MenuState::ButtonStruct({
				button.floats.at("xPos"),
				button.floats.at("yPos"),
				button.floats.at("xTexStart"),
				button.floats.at("yTexStart"),
				button.floats.at("xTexEnd"),
				button.floats.at("yTexEnd"),
				button.floats.at("height"),
				button.floats.at("width"),
				button.strings.at("texture"),
				functions.at(testinging) //crap used until the buggy file handler is fixed
			});
		}
	}

	//Gather all the Menus in this vector for fututre use
	std::vector<Menu> menuGather;
	for (auto const& menu : menuFile)
	{
		//If it is a menu add one to the vector
		if (menu.ints.find("State") != menu.ints.end())
		{
			//Temporary Button Vector until Menu has been given them
			std::vector<MenuState::ButtonStruct> tempButton;
			for (int i = 0; i < menu.ints.at("buttonAmmount"); i++)
			{
				tempButton.push_back(allButtons.at(menu.strings.at("button" + std::to_string(i + 1))));
			}
			menuGather.push_back({
				menu.ints.at("State"),
				menu.strings.at("Background"),
				tempButton
			});
		}
	}

	//Create new Menus and send in the fitting information from Menu vector
	for (auto const& menus : menuGather)
	{
		m_menuStates[GameState(menus.theState)] = newd MenuState();
		m_menuStates.at(GameState(menus.theState))->initialize(menus.buttons, menus.background);

	}
	/*MenuState* test = newd MenuState();
	test->initialize(std::bind(&MenuMachine::buttonClick0, this));
	m_menuStates[gameStateMenuMain] = test;

	MenuState* test1 = newd MenuState();
	test1->initialize(std::bind(&MenuMachine::buttonClick1, this));
	m_menuStates[gameStateMenuSettings] = test1;

	MenuState* test2 = newd MenuState();
	test2->initialize(std::bind(&MenuMachine::buttonClick2, this));
	m_menuStates[gameStateGame] = test2;*/

	showMenu(state);

	//accquire menu state
	//accquires position
	//accquires its width and height
	//accquires texture
}

void Logic::MenuMachine::clear() 
{
	currentActiveMenu = nullptr;
}

void Logic::MenuMachine::update()
{
	auto Mouse = DirectX::Mouse::Get().GetState();

	if (Mouse.leftButton && !pressed)
	{
		pressed = true;
		currentActiveMenu->updateOnPress(Mouse.x, Mouse.y);
	}
	else if (!Mouse.leftButton && pressed)
	{
		pressed = false;

	}
}

void Logic::MenuMachine::showMenu(GameState state)
{
	if (m_menuStates.find(state) != m_menuStates.end())
	{
		currentActiveMenu = m_menuStates.at(state);
		currentActiveState = state;
	}
	else
	{
		currentActiveMenu = m_menuStates.at(gameStateMenuMain); //change to error state
		currentActiveState = gameStateMenuMain;
	}
}

GameState Logic::MenuMachine::currentState()
{
	return currentActiveState;
}

void Logic::MenuMachine::buttonClick0()
{
	showMenu(gameStateMenuSettings);
	std::cout << "Left Trigger: Switched To Menu State 1";
}

void Logic::MenuMachine::buttonClick1()
{
	showMenu(gameStateGame);
	std::cout << "Left Trigger: Switched To Menu State 2";
}

void Logic::MenuMachine::buttonClick2()
{
	showMenu(gameStateMenuMain);
	std::cout << "Left Trigger: Switched To Menu State 0";
}
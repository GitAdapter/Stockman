#include <Misc/CardManager.h>
#include <Player/Player.h>
using namespace Logic;

const int CardManager::HEALTH_PACK = 0;
const int CardManager::HAND_SIZE   = 3;
const int CardManager::NEVER_REMOVE_CARDS = 1;

CardManager::CardManager(int nrOfEach)
{
    m_hand.resize(HAND_SIZE);
    memset(m_hand.data(), -1, sizeof(m_hand.data()));

    loadDeckFromFile();
    createDeck(nrOfEach);
}

CardManager::~CardManager() { }

void CardManager::resetDeck()
{
    for (auto &pair : m_deck)
        if (pair.first == TAKEN)
            pair.first = IN_DECK;
}

void CardManager::createDeck(int nrOfEach)
{
    m_deck.push_back({ NEVER_REMOVE, HEALTH_PACK });
	for (int i = 0; i < nrOfEach; i++)
		for (int j = 1; j < m_cards.size(); j++)
            m_deck.push_back({ IN_DECK, j });
}

void CardManager::pickThreeCards(bool damaged)
{
    currenthand.clear();
    int cardsPicked = 0;
    int amount = HAND_SIZE;

    if (damaged)
    {
        m_hand[HAND_SIZE - 1] = HEALTH_PACK;
        amount--;
    }
    
    shuffle();
    for (int i = 0; i < m_deck.size() && cardsPicked < amount; i++)
        if (m_deck[i].first == IN_DECK)
            m_hand[cardsPicked++] = i;

    if (cardsPicked < amount)
        throw std::runtime_error("Not enough cards");

    pepperCardsForDraw();
}

void CardManager::shuffle()
{
    static std::random_device rd;
    static std::mt19937 g(rd());
    std::shuffle(m_deck.begin() + NEVER_REMOVE_CARDS, m_deck.end(), g);
}

void Logic::CardManager::pepperCardsForDraw()
{
    currenthand.clear();
    for (size_t i = 0; i < HAND_SIZE; i++)
    {
        currenthand.push_back(m_cards[m_deck[m_hand.at(i)].second]);
        currenthand.at(i).setIconPos(365.0f + 250 * i, 260.0f, 75, 75);
        currenthand.at(i).setbackgroundPos(290.f + 250 * i, 225.0f, 200, 250);
        currenthand.at(i).setCardTextPos(308.f + 250.f * i);
    }
}

Card CardManager::pick(int handIndex)
{
    int deckIndex = m_hand[handIndex];

    if (m_deck[deckIndex].first == IN_DECK)
        m_deck[deckIndex].first = TAKEN;

    return m_cards[m_deck[deckIndex].second];
}

void Logic::CardManager::render() const
{
    for (auto cards : currenthand)
    {
        cards.render();
    }
}

void CardManager::loadDeckFromFile()
{
    std::vector<FileLoader::LoadedStruct> cardFile;
    FileLoader::singleton().loadStructsFromFile(cardFile, "Cards");

    createCard(NEVER_REMOVE, cardFile[0]);
    for (int i = 1; i < cardFile.size(); i++)
        createCard(IN_DECK, cardFile[i]);
}

void CardManager::createCard(CardCondition cond, FileLoader::LoadedStruct const &struc)
{
    std::vector<int> statusIds;
    for (int i = 0; i < struc.ints.at("statusAmount"); i++)
        statusIds.push_back(struc.ints.at("status" + std::to_string(i)));

    DirectX::SimpleMath::Vector2 texStart(struc.floats.at("xTexStart"),
        struc.floats.at("yTexStart"));
    DirectX::SimpleMath::Vector2 texEnd(struc.floats.at("xTexEnd"),
        struc.floats.at("yTexEnd"));;
    
    m_cards.push_back(Card(struc.strings.at("cardName"),
        struc.strings.at("texture"), struc.strings.at("description"),
        statusIds, texStart, texEnd, struc.ints.at("statusType"), struc.ints.at("category"))
    ); 
}

bool CardManager::pickAndApplyCard(Player & player, int cardIndex)
{
    if (cardIndex >= 0)
    {
        Card temp = pick(cardIndex);
        handleCard(player, temp);
        return true;
    }
    return false;
}

void CardManager::handleCard(Player &player, Card const &card)
{
    //add information to player
    for (auto const& ID : card.getStatusIds())
    {
        switch (card.getStatusType())
        {
            case Card::EFFECT:
                player.getStatusManager().addStatusResetDuration(
                    static_cast<StatusManager::EFFECT_ID> (ID),
                    1
                );
                break;
            case Card::UPGRADE:
                player.upgrade(static_cast<StatusManager::UPGRADE_ID> (ID));
                break;
            default:
                printf("Unsupported card type");
                break;
        }
    }
}

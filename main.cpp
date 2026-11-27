#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>

extern "C" {
    #include "network.h"
    #include "graph.h"
    #include "dijkstra.h"
}

class Router : public sf::CircleShape {
private:
    int id;
    bool working;
    sf::Vector2f worldPosition;
    float worldRadius;

public:
    Router(int routerId, float x, float y, float radius, bool on)
        : sf::CircleShape(radius), id(routerId), working(on),
          worldPosition(x,y), worldRadius(radius)
    {
        setPointCount(80);
        if(working)
            setFillColor(sf::Color(91,165,155));
        else
            setFillColor(sf::Color(90,102,115));
    }

    int getId() const {
        return id;
    }

    void updateScreen(const sf::FloatRect& canvas) {
        const float WORLD_SIZE = 10000.f;
        const float WORLD_PADDING = 400.f;
        const float SCREEN_MARGIN = 24.f;
        float totalWorldSize = WORLD_SIZE + 2.f * WORLD_PADDING;

        float scale = std::min(
            (canvas.size.x - 2.f * SCREEN_MARGIN) / totalWorldSize,
            (canvas.size.y - 2.f * SCREEN_MARGIN) / totalWorldSize
        );

        float displaySize = totalWorldSize * scale;
        float offsetX = canvas.position.x + (canvas.size.x - displaySize) / 2.f;
        float offsetY = canvas.position.y + (canvas.size.y - displaySize) / 2.f;
        float screenX = offsetX + (worldPosition.x + WORLD_PADDING) * scale;
        float screenY = offsetY + (worldPosition.y + WORLD_PADDING) * scale;
        float screenRadius = std::max(1.f,worldRadius * scale);

        setRadius(screenRadius);
        setOrigin({screenRadius,screenRadius});
        setPosition({screenX,screenY});
    }
};

struct Asset {
    std::unique_ptr<sf::Texture> texture;
    int insetPx = 0;
};

enum class FocusedInput {
    None,
    Source,
    Destination,
    Message
};

enum class LogLevel {
    Info,
    Success,
    Warning,
    Error
};

struct LogEntry {
    std::string message;
    LogLevel level;
};

std::unordered_map<std::string, sf::FloatRect> UI_RECT;
std::unordered_map<std::string, Asset> UI_ASSETS;
std::vector<Router> routers;
std::vector<int> shortestPath;
std::vector<LogEntry> eventLog;

int selectedRouterId = -1;
int sourceRouterId = -1;
int destinationRouterId = -1;
FocusedInput focusedInput = FocusedInput::None;

std::string sourceText;
std::string destinationText;
std::string messageText;

double shortestPathCost = 0.0;
bool packetMoving = false;
std::size_t packetSegment = 0;
float packetProgress = 0.f;
const float HOP_DURATION = 0.55f;

const sf::Color WINDOW_BG(8,16,24);
const sf::Color TEXT_PRIMARY(230,236,242);
const sf::Color TEXT_MUTED(165,178,190);
const sf::Color LINK_COLOR(70,110,140,150);
const sf::Color PATH_COLOR(62,212,194);
const sf::Color SELECTED_COLOR(255,183,65);
const sf::Color SOURCE_COLOR(75,205,150);
const sf::Color DESTINATION_COLOR(110,155,255);
const sf::Color ERROR_COLOR(242,130,126);
const float ASSET_SCALE = 4.f;

void addLog(const std::string& message, LogLevel level = LogLevel::Info) {
    eventLog.push_back({message,level});
    if(eventLog.size() > 10)
        eventLog.erase(eventLog.begin());
    std::cout << message << '\n';
}

void resetRoute() {
    shortestPath.clear();
    shortestPathCost = 0.0;
    packetMoving = false;
    packetSegment = 0;
    packetProgress = 0.f;
}

void updateRouterLayout() {
    const auto& canvas = UI_RECT.at("canvas");
    for(auto& router : routers)
        router.updateScreen(canvas);
}

void syncRouters() {
    routers.clear();
    for(int i = 1; i <= getNetworkCount(); i++) {
        Network network = getNetwork(i);
        routers.emplace_back(network.id,network.x,network.y,network.radius,network.working);
    }
    updateRouterLayout();
}

const Router* findRouter(int id) {
    for(const auto& router : routers) {
        if(router.getId() == id)
            return &router;
    }
    return nullptr;
}

bool loadAsset(const std::string& name, int insetPx = 0) {
    auto texture = std::make_unique<sf::Texture>();
    if(!texture->loadFromFile("assets/ui/" + name + ".png")) {
        std::cerr << "Cannot load assets/ui/" << name << ".png\n";
        return false;
    }
    texture->setSmooth(true);
    UI_ASSETS.emplace(name,Asset{std::move(texture),insetPx});
    return true;
}

bool loadUIAssets() {
    return loadAsset("main-panel",24) &&
           loadAsset("sidebar-panel",24) &&
           loadAsset("status-panel",4) &&
           loadAsset("networklab-logo") &&
           loadAsset("button",16) &&
           loadAsset("button-hover",16) &&
           loadAsset("button-active",36) &&
           loadAsset("button-pressed",16) &&
           loadAsset("button-disabled",16) &&
           loadAsset("input-field",16) &&
           loadAsset("input-focused",16) &&
           loadAsset("source-indicator") &&
           loadAsset("destination-indicator") &&
           loadAsset("status-active") &&
           loadAsset("status-offline") &&
           loadAsset("packet-indicator");
}

void updateLayout(float width, float height) {
    float sidebarWidth = width * 0.25f;
    float footerHeight = height * 0.10f;
    float canvasWidth = width - sidebarWidth;
    float contentHeight = height - footerHeight;

    float padding = sidebarWidth * 0.08f;
    float gap = 12.f;
    float left = canvasWidth + padding;
    float availableWidth = sidebarWidth - 2.f * padding;
    float halfWidth = (availableWidth - gap) / 2.f;
    float controlHeight = 42.f;

    UI_RECT["canvas"] = {{0.f,0.f},{canvasWidth,contentHeight}};
    UI_RECT["sidebar"] = {{canvasWidth,0.f},{sidebarWidth,contentHeight}};
    UI_RECT["footer"] = {{0.f,contentHeight},{width,footerHeight}};

    UI_RECT["add_router"] = {{left,contentHeight * 0.165f},{halfWidth,controlHeight}};
    UI_RECT["remove_router"] = {{left + halfWidth + gap,contentHeight * 0.165f},{halfWidth,controlHeight}};
    UI_RECT["toggle_router"] = {{left,contentHeight * 0.25f},{availableWidth,controlHeight}};
    UI_RECT["selected_router"] = {{left,contentHeight * 0.345f},{availableWidth,66.f}};

    UI_RECT["source"] = {{left,contentHeight * 0.465f},{halfWidth,controlHeight}};
    UI_RECT["destination"] = {{left + halfWidth + gap,contentHeight * 0.465f},{halfWidth,controlHeight}};
    UI_RECT["run_dijkstra"] = {{left,contentHeight * 0.56f},{availableWidth,controlHeight}};

    UI_RECT["message_input"] = {{left,contentHeight * 0.73f},{availableWidth,controlHeight}};
    UI_RECT["send_message"] = {{left,contentHeight * 0.82f},{halfWidth,controlHeight}};
    UI_RECT["reset_network"] = {{left + halfWidth + gap,contentHeight * 0.82f},{halfWidth,controlHeight}};

    float logoWidth = std::min(190.f,availableWidth);
    float logoHeight = logoWidth * 24.f / 190.f;
    UI_RECT["logo"] = {{left,25.f},{logoWidth,logoHeight}};
}

void drawTile(sf::RenderWindow& window,const sf::Texture& texture,
              int sx,int sy,int sw,int sh,float dx,float dy,float dw,float dh) {
    if(sw <= 0 || sh <= 0 || dw <= 0.f || dh <= 0.f)
        return;

    sf::Sprite sprite(texture,sf::IntRect({sx,sy},{sw,sh}));
    sprite.setPosition({dx,dy});
    sprite.setScale({dw / static_cast<float>(sw),dh / static_cast<float>(sh)});
    window.draw(sprite);
}

void drawAsset(sf::RenderWindow& window,const std::string& name,const sf::FloatRect& rect) {
    const Asset& asset = UI_ASSETS.at(name);
    const sf::Texture& texture = *asset.texture;
    auto size = texture.getSize();
    int tw = static_cast<int>(size.x);
    int th = static_cast<int>(size.y);
    float x = rect.position.x;
    float y = rect.position.y;
    float w = rect.size.x;
    float h = rect.size.y;

    if(asset.insetPx == 0) {
        drawTile(window,texture,0,0,tw,th,x,y,w,h);
        return;
    }

    int cornerPx = asset.insetPx;
    float corner = static_cast<float>(cornerPx) / ASSET_SCALE;
    float left = std::min(corner,w / 2.f);
    float top = std::min(corner,h / 2.f);
    float right = left;
    float bottom = top;

    int sx[4] = {0,cornerPx,tw-cornerPx,tw};
    int sy[4] = {0,cornerPx,th-cornerPx,th};
    float dx[4] = {x,x+left,x+w-right,x+w};
    float dy[4] = {y,y+top,y+h-bottom,y+h};

    for(int row = 0; row < 3; row++) {
        for(int col = 0; col < 3; col++) {
            drawTile(window,texture,
                     sx[col],sy[row],sx[col+1]-sx[col],sy[row+1]-sy[row],
                     dx[col],dy[row],dx[col+1]-dx[col],dy[row+1]-dy[row]);
        }
    }
}

void drawText(sf::RenderWindow& window,const sf::Font& font,
              const std::string& message,const sf::FloatRect& rect,
              unsigned int fontSize,const sf::Color& color) {
    sf::Text text(font,message,fontSize);
    text.setFillColor(color);
    auto bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                    bounds.position.y + bounds.size.y / 2.f});
    text.setPosition({rect.position.x + rect.size.x / 2.f,
                      rect.position.y + rect.size.y / 2.f});
    window.draw(text);
}

void drawLeftText(sf::RenderWindow& window,const sf::Font& font,
                  const std::string& message,float x,float y,
                  unsigned int fontSize,const sf::Color& color) {
    sf::Text text(font,message,fontSize);
    text.setFillColor(color);
    text.setPosition({x,y});
    window.draw(text);
}

sf::Color logColor(LogLevel level) {
    if(level == LogLevel::Success) return PATH_COLOR;
    if(level == LogLevel::Warning) return SELECTED_COLOR;
    if(level == LogLevel::Error) return ERROR_COLOR;
    return TEXT_MUTED;
}

void drawEventLog(sf::RenderWindow& window,const sf::Font& font) {
    const auto& footer = UI_RECT.at("footer");
    float x = std::min(355.f,footer.size.x * 0.32f);
    float y = footer.position.y + 8.f;
    float lineHeight = 18.f;
    int maxLines = std::max(1,std::min(4,static_cast<int>((footer.size.y - 12.f) / lineHeight)));

    int count = std::min(maxLines,static_cast<int>(eventLog.size()));
    int first = static_cast<int>(eventLog.size()) - count;

    for(int i = 0; i < count; i++) {
        const auto& entry = eventLog[first + i];
        drawLeftText(window,font,entry.message,x,y + i * lineHeight,12,logColor(entry.level));
    }
}

void drawBackgrounds(sf::RenderWindow& window) {
    drawAsset(window,"main-panel",UI_RECT.at("canvas"));
    drawAsset(window,"sidebar-panel",UI_RECT.at("sidebar"));
    drawAsset(window,"status-panel",UI_RECT.at("footer"));
    drawAsset(window,"networklab-logo",UI_RECT.at("logo"));
}

void drawControls(sf::RenderWindow& window,const sf::Font& font,sf::Vector2f mouse) {
    const std::string buttons[] = {
        "add_router","remove_router","toggle_router","run_dijkstra","send_message","reset_network"
    };

    for(const std::string& name : buttons) {
        const auto& rect = UI_RECT.at(name);
        std::string assetName = "button";
        if(rect.contains(mouse))
            assetName = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)
                        ? "button-pressed" : "button-hover";
        else if(name == "run_dijkstra")
            assetName = "button-active";
        drawAsset(window,assetName,rect);
    }

    drawAsset(window,focusedInput == FocusedInput::Source ? "input-focused" : "input-field",UI_RECT.at("source"));
    drawAsset(window,focusedInput == FocusedInput::Destination ? "input-focused" : "input-field",UI_RECT.at("destination"));
    drawAsset(window,focusedInput == FocusedInput::Message ? "input-focused" : "input-field",UI_RECT.at("message_input"));

    drawText(window,font,"Add",UI_RECT.at("add_router"),18,TEXT_PRIMARY);
    drawText(window,font,"Remove",UI_RECT.at("remove_router"),18,TEXT_PRIMARY);
    drawText(window,font,"Enable / Disable",UI_RECT.at("toggle_router"),17,TEXT_PRIMARY);

    drawText(window,font,sourceText.empty() ? "Source" : sourceText,UI_RECT.at("source"),17,
             sourceText.empty() ? TEXT_MUTED : (sourceRouterId == -1 ? ERROR_COLOR : TEXT_PRIMARY));
    drawText(window,font,destinationText.empty() ? "Destination" : destinationText,UI_RECT.at("destination"),17,
             destinationText.empty() ? TEXT_MUTED : (destinationRouterId == -1 ? ERROR_COLOR : TEXT_PRIMARY));
    drawText(window,font,"Run Dijkstra",UI_RECT.at("run_dijkstra"),18,TEXT_PRIMARY);

    std::string visibleMessage = messageText;
    if(visibleMessage.size() > 27)
        visibleMessage = "..." + visibleMessage.substr(visibleMessage.size() - 24);
    drawText(window,font,visibleMessage.empty() ? "Message" : visibleMessage,UI_RECT.at("message_input"),16,
             visibleMessage.empty() ? TEXT_MUTED : TEXT_PRIMARY);
    drawText(window,font,"Send",UI_RECT.at("send_message"),18,TEXT_PRIMARY);
    drawText(window,font,"Reset",UI_RECT.at("reset_network"),18,TEXT_PRIMARY);
}

void drawSelectedRouterPanel(sf::RenderWindow& window,const sf::Font& font) {
    const auto& rect = UI_RECT.at("selected_router");
    sf::RectangleShape panel(rect.size);
    panel.setPosition(rect.position);
    panel.setFillColor(sf::Color(29,45,52));
    panel.setOutlineColor(sf::Color(182,154,100));
    panel.setOutlineThickness(1.f);
    window.draw(panel);

    sf::FloatRect heading = {{rect.position.x + 8.f,rect.position.y + 4.f},{rect.size.x - 16.f,25.f}};
    drawText(window,font,"SELECTED ROUTER",heading,13,sf::Color(195,169,116));
    std::string id = selectedRouterId == -1 ? "--" : "R" + std::to_string(selectedRouterId);
    sf::FloatRect value = {{rect.position.x + 8.f,rect.position.y + 30.f},{rect.size.x - 16.f,30.f}};
    drawText(window,font,id,value,20,sf::Color(235,202,145));
}

void drawUI(sf::RenderWindow& window,const sf::Font& font) {
    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    drawBackgrounds(window);
    const auto& sidebar = UI_RECT.at("sidebar");
    float headingX = sidebar.position.x + sidebar.size.x * 0.08f;
    float headingHeight = sidebar.size.y;

    drawLeftText(window,font,"NETWORK",headingX,headingHeight * 0.18f - 33.f,15,TEXT_MUTED);
    drawLeftText(window,font,"ROUTING",headingX,headingHeight * 0.45f - 33.f,15,TEXT_MUTED);
    drawLeftText(window,font,"MESSAGE",headingX,headingHeight * 0.73f - 33.f,15,TEXT_MUTED);
    drawControls(window,font,mouse);
    drawSelectedRouterPanel(window,font);

    const auto& footer = UI_RECT.at("footer");
    drawLeftText(window,font,"Network Simulator",24.f,footer.position.y + 12.f,22,TEXT_PRIMARY);
    drawLeftText(window,font,"C engine  /  SFML visualizer",25.f,footer.position.y + 44.f,14,TEXT_MUTED);
    drawEventLog(window,font);
}

void drawConnection(sf::RenderWindow& window,const sf::Vector2f& start,
                    const sf::Vector2f& end,float thickness,const sf::Color& color) {
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float length = std::sqrt(dx * dx + dy * dy);
    if(length < 0.001f)
        return;

    float angleDeg = std::atan2(dy,dx) * 180.f / 3.14159265f;
    sf::RectangleShape line({length,thickness});
    line.setPosition(start);
    line.setOrigin({0.f,thickness / 2.f});
    line.setRotation(sf::degrees(angleDeg));
    line.setFillColor(color);
    window.draw(line);
}

void drawConnections(sf::RenderWindow& window) {
    for(int i = 1; i <= getNetworkCount(); i++) {
        for(int j = i + 1; j <= getNetworkCount(); j++) {
            if(!hasConnection(i,j))
                continue;

            const Router* first = findRouter(i);
            const Router* second = findRouter(j);
            if(first && second)
                drawConnection(window,first->getPosition(),second->getPosition(),2.f,LINK_COLOR);
        }
    }
}

void drawShortestPath(sf::RenderWindow& window) {
    for(std::size_t i = 1; i < shortestPath.size(); i++) {
        const Router* first = findRouter(shortestPath[i-1]);
        const Router* second = findRouter(shortestPath[i]);
        if(first && second)
            drawConnection(window,first->getPosition(),second->getPosition(),4.f,PATH_COLOR);
    }
}

void drawLinkCostTooltip(sf::RenderWindow& window,const sf::Font& font) {
    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    const auto& canvas = UI_RECT.at("canvas");
    if(!canvas.contains(mouse))
        return;

    for(const auto& router : routers) {
        float dx = mouse.x - router.getPosition().x;
        float dy = mouse.y - router.getPosition().y;
        if(dx * dx + dy * dy < router.getRadius() * router.getRadius())
            return;
    }

    int fromId = -1;
    int toId = -1;
    float bestDistanceSquared = 8.f * 8.f;

    for(int i = 1; i <= getNetworkCount(); i++) {
        for(int j = i + 1; j <= getNetworkCount(); j++) {
            if(!hasConnection(i,j))
                continue;

            const Router* first = findRouter(i);
            const Router* second = findRouter(j);
            if(!first || !second)
                continue;

            sf::Vector2f a = first->getPosition();
            sf::Vector2f b = second->getPosition();
            float dx = b.x - a.x;
            float dy = b.y - a.y;
            float lengthSquared = dx * dx + dy * dy;
            if(lengthSquared < 0.001f)
                continue;

            float t = ((mouse.x - a.x) * dx + (mouse.y - a.y) * dy) / lengthSquared;
            t = std::clamp(t,0.f,1.f);
            float closestX = a.x + t * dx;
            float closestY = a.y + t * dy;
            float distanceX = mouse.x - closestX;
            float distanceY = mouse.y - closestY;
            float distanceSquared = distanceX * distanceX + distanceY * distanceY;

            if(distanceSquared < bestDistanceSquared) {
                bestDistanceSquared = distanceSquared;
                fromId = i;
                toId = j;
            }
        }
    }

    if(fromId == -1)
        return;

    const Router* first = findRouter(fromId);
    const Router* second = findRouter(toId);
    if(!first || !second)
        return;

    drawConnection(window,first->getPosition(),second->getPosition(),3.f,
                   sf::Color(202,183,127,230));

    Link link = getLink(fromId,toId);
    std::ostringstream label;
    label << "R" << fromId << " - R" << toId << "   Cost: "
          << std::fixed << std::setprecision(1) << link.cost;

    float width = 225.f;
    float height = 34.f;
    float left = std::clamp(mouse.x + 15.f,canvas.position.x + 5.f,
                            std::max(canvas.position.x + 5.f,canvas.position.x + canvas.size.x - width - 5.f));
    float top = std::clamp(mouse.y - 46.f,canvas.position.y + 5.f,
                           std::max(canvas.position.y + 5.f,canvas.position.y + canvas.size.y - height - 5.f));

    sf::RectangleShape tooltip({width,height});
    tooltip.setPosition({left,top});
    tooltip.setFillColor(sf::Color(21,37,49,245));
    tooltip.setOutlineColor(sf::Color(135,156,164,220));
    tooltip.setOutlineThickness(1.f);
    window.draw(tooltip);
    drawText(window,font,label.str(),{{left + 5.f,top},{width - 10.f,height}},14,TEXT_PRIMARY);
}

void drawRouters(sf::RenderWindow& window) {
    for(const auto& router : routers)
        window.draw(router);
}

void drawRouterIDs(sf::RenderWindow& window,const sf::Font& font) {
    for(const auto& router : routers) {
        sf::Text label(font,"R" + std::to_string(router.getId()),12);
        label.setFillColor(sf::Color(170,190,200));
        sf::Vector2f position = router.getPosition();
        float radius = router.getRadius();
        label.setPosition({position.x + radius + 5.f,position.y - radius - 7.f});
        window.draw(label);
    }
}

void drawRouterRing(sf::RenderWindow& window,int routerId,const sf::Color& color,float offset = 4.f) {
    const Router* router = findRouter(routerId);
    if(!router)
        return;

    float radius = router->getRadius() + offset;
    sf::CircleShape ring(radius);
    ring.setPointCount(80);
    ring.setOrigin({radius,radius});
    ring.setPosition(router->getPosition());
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineColor(color);
    ring.setOutlineThickness(2.5f);
    window.draw(ring);
}

void drawPacket(sf::RenderWindow& window) {
    if(!packetMoving || packetSegment + 1 >= shortestPath.size())
        return;

    const Router* first = findRouter(shortestPath[packetSegment]);
    const Router* second = findRouter(shortestPath[packetSegment+1]);
    if(!first || !second)
        return;

    sf::Vector2f start = first->getPosition();
    sf::Vector2f end = second->getPosition();
    sf::Vector2f point = start + (end - start) * packetProgress;

    sf::CircleShape glow(10.f);
    glow.setOrigin({10.f,10.f});
    glow.setPosition(point);
    glow.setFillColor(sf::Color(255,191,82,55));
    window.draw(glow);

    sf::CircleShape packet(5.f);
    packet.setPointCount(32);
    packet.setOrigin({5.f,5.f});
    packet.setPosition(point);
    packet.setFillColor(sf::Color(255,214,123));
    window.draw(packet);
}

void clearInputsAfterRemoval() {
    selectedRouterId = -1;
    sourceRouterId = -1;
    destinationRouterId = -1;
    sourceText.clear();
    destinationText.clear();
    focusedInput = FocusedInput::None;
    resetRoute();
}

void addNetwork() {
    if(!createNewConnection()) {
        addLog("ERROR: Could not create router (maximum 40 or no space)",LogLevel::Error);
        return;
    }

    int newId = getNetworkCount();
    addNewConnection(newId);
    syncRouters();
    resetRoute();
    addLog("Router R" + std::to_string(newId) + " added",LogLevel::Success);
}

void removeNetwork() {
    if(!valid(selectedRouterId)) {
        addLog("ERROR: Select a router to remove",LogLevel::Error);
        return;
    }

    int removedId = selectedRouterId;
    if(!::removeNetwork(removedId)) {
        addLog("ERROR: Could not remove R" + std::to_string(removedId),LogLevel::Error);
        return;
    }

    syncRouters();
    clearInputsAfterRemoval();
    addLog("Router R" + std::to_string(removedId) + " removed; IDs refreshed",LogLevel::Warning);
}

void toggleRouter() {
    if(!valid(selectedRouterId)) {
        addLog("ERROR: Select a router to enable or disable",LogLevel::Error);
        return;
    }

    Network* holder = getHolder();
    Network& network = holder[selectedRouterId - 1];
    network.working = !network.working;
    bool working = network.working;
    int id = selectedRouterId;

    syncRouters();
    resetRoute();
    addLog("Router R" + std::to_string(id) + (working ? " enabled" : " disabled"),LogLevel::Warning);
}

std::string pathDescription(const std::vector<int>& path) {
    std::string result;
    for(std::size_t i = 0; i < path.size(); i++) {
        if(i > 0)
            result += " -> ";
        result += "R" + std::to_string(path[i]);
    }
    return result;
}

bool runDijkstra() {
    resetRoute();

    if(!valid(sourceRouterId) || !valid(destinationRouterId)) {
        addLog("ERROR: Enter valid Source and Destination router IDs",LogLevel::Error);
        return false;
    }
    if(!getNetwork(sourceRouterId).working || !getNetwork(destinationRouterId).working) {
        addLog("ERROR: Source and Destination must be online",LogLevel::Error);
        return false;
    }

    PathResult result = findShortestPath(sourceRouterId,destinationRouterId);
    if(result.count <= 0 || result.count > 40) {
        addLog("ERROR: No route from R" + std::to_string(sourceRouterId) + " to R" + std::to_string(destinationRouterId),LogLevel::Error);
        return false;
    }

    for(int i = 0; i < result.count; i++) {
        int id = result.paths[i];
        if(!valid(id) || !getNetwork(id).working) {
            addLog("ERROR: Route includes an offline router",LogLevel::Error);
            resetRoute();
            return false;
        }
        if(i > 0) {
            int previous = result.paths[i - 1];
            if(!hasConnection(previous,id)) {
                addLog("ERROR: Engine returned a path containing a missing link",LogLevel::Error);
                resetRoute();
                return false;
            }
            shortestPathCost += getLink(previous,id).cost;
        }
        shortestPath.push_back(id);
    }

    std::ostringstream cost;
    cost << std::fixed << std::setprecision(1) << shortestPathCost;
    addLog("Route: " + pathDescription(shortestPath),LogLevel::Success);
    addLog("Hops: " + std::to_string(shortestPath.size()-1) + " | Total cost: " + cost.str(),LogLevel::Success);
    return true;
}

void sendMessage() {
    if(messageText.empty()) {
        addLog("ERROR: Type a message first",LogLevel::Error);
        return;
    }
    if(packetMoving) {
        addLog("Packet already in transit",LogLevel::Warning);
        return;
    }
    if(shortestPath.empty() && !runDijkstra())
        return;

    if(shortestPath.size() == 1) {
        addLog("Message delivered locally at R" + std::to_string(shortestPath.front()),LogLevel::Success);
        return;
    }

    packetSegment = 0;
    packetProgress = 0.f;
    packetMoving = true;
    addLog("Sending \"" + messageText + "\" from R" + std::to_string(shortestPath.front()),LogLevel::Info);
}

void updatePacket(float deltaTime) {
    if(!packetMoving)
        return;

    packetProgress += deltaTime / HOP_DURATION;
    while(packetProgress >= 1.f && packetMoving) {
        packetProgress -= 1.f;
        packetSegment++;
        int reachedId = shortestPath[packetSegment];
        addLog("Packet reached R" + std::to_string(reachedId),LogLevel::Info);

        if(packetSegment + 1 >= shortestPath.size()) {
            packetMoving = false;
            packetProgress = 0.f;
            addLog("Message delivered successfully",LogLevel::Success);
        }
    }
}

void updateTypedId(FocusedInput input) {
    std::string& text = input == FocusedInput::Source ? sourceText : destinationText;
    int& routerId = input == FocusedInput::Source ? sourceRouterId : destinationRouterId;
    routerId = -1;

    if(!text.empty()) {
        int id = std::stoi(text);
        if(valid(id))
            routerId = id;
    }
    resetRoute();
}

void handleTextInput(char32_t unicode) {
    if(focusedInput == FocusedInput::None)
        return;

    if(focusedInput == FocusedInput::Message) {
        if(unicode == 8) {
            if(!messageText.empty())
                messageText.pop_back();
        }
        else if(unicode >= 32 && unicode <= 126 && messageText.size() < 48)
            messageText.push_back(static_cast<char>(unicode));
        return;
    }

    std::string& text = focusedInput == FocusedInput::Source ? sourceText : destinationText;
    if(unicode == 8) {
        if(!text.empty())
            text.pop_back();
    }
    else if(unicode >= U'0' && unicode <= U'9') {
        if(text.size() < 2)
            text.push_back(static_cast<char>(unicode));
    }
    else
        return;

    updateTypedId(focusedInput);
}

bool generateTopology();

void resetNetwork() {
    while(getNetworkCount() > 0) {
        if(!::removeNetwork(getNetworkCount())) {
            addLog("ERROR: Could not reset the C network",LogLevel::Error);
            return;
        }
    }

    selectedRouterId = -1;
    sourceRouterId = -1;
    destinationRouterId = -1;
    focusedInput = FocusedInput::None;
    sourceText.clear();
    destinationText.clear();
    messageText.clear();
    resetRoute();
    eventLog.clear();

    if(!generateTopology()) {
        syncRouters();
        addLog("ERROR: Could not generate a new topology",LogLevel::Error);
        return;
    }

    syncRouters();
    addLog("Network reset: " + std::to_string(getNetworkCount()) + " routers",LogLevel::Success);
}

void handleMouseClick(sf::Vector2f mouse) {
    if(UI_RECT.at("source").contains(mouse)) {
        focusedInput = FocusedInput::Source;
        return;
    }
    if(UI_RECT.at("destination").contains(mouse)) {
        focusedInput = FocusedInput::Destination;
        return;
    }
    if(UI_RECT.at("message_input").contains(mouse)) {
        focusedInput = FocusedInput::Message;
        return;
    }

    focusedInput = FocusedInput::None;
    if(UI_RECT.at("add_router").contains(mouse)) {
        addNetwork();
        return;
    }
    if(UI_RECT.at("remove_router").contains(mouse)) {
        removeNetwork();
        return;
    }
    if(UI_RECT.at("toggle_router").contains(mouse)) {
        toggleRouter();
        return;
    }
    if(UI_RECT.at("run_dijkstra").contains(mouse)) {
        runDijkstra();
        return;
    }
    if(UI_RECT.at("send_message").contains(mouse)) {
        sendMessage();
        return;
    }
    if(UI_RECT.at("reset_network").contains(mouse)) {
        resetNetwork();
        return;
    }

    if(!UI_RECT.at("canvas").contains(mouse))
        return;

    selectedRouterId = -1;
    for(const auto& router : routers) {
        sf::Vector2f center = router.getPosition();
        float dx = mouse.x - center.x;
        float dy = mouse.y - center.y;
        float radius = router.getRadius();
        if(dx * dx + dy * dy <= radius * radius) {
            selectedRouterId = router.getId();
            addLog("Selected R" + std::to_string(selectedRouterId));
            return;
        }
    }
}

bool createRouters() {
    for(int i = 0; i < 20; i++) {
        if(!createNewConnection()) {
            std::cerr << "FAIL: Network creation at " << i + 1 << '\n';
            return false;
        }
    }
    return true;
}

bool generateTopology() {
    initializeGraph();
    if(!createRouters())
        return false;

    int added = 0;
    for(int tries = 0; tries < 5000 && added < 15; tries++) {
        if(addConnection())
            added++;
    }
    return true;
}

int main() {
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode({1500,900}),"NetworkLab",sf::Style::Default,sf::State::Windowed,settings);
    window.setFramerateLimit(60);

    srand(static_cast<unsigned int>(time(nullptr)));
    if(!generateTopology())
        return 1;

    sf::Font font;
    if(!font.openFromFile("assets/fonts/Inter-Medium.ttf")) {
        std::cerr << "Cannot load assets/fonts/Inter-Medium.ttf\n";
        return 1;
    }
    if(!loadUIAssets()) {
        UI_ASSETS.clear();
        return 1;
    }

    updateLayout(static_cast<float>(window.getSize().x),static_cast<float>(window.getSize().y));
    syncRouters();
    addLog("Network ready: " + std::to_string(getNetworkCount()) + " routers",LogLevel::Success);
    addLog("Select a router or type Source and Destination IDs");

    sf::Clock frameClock;
    while(window.isOpen()) {
        float deltaTime = std::min(frameClock.restart().asSeconds(),0.1f);

        while(const std::optional event = window.pollEvent()) {
            if(event->is<sf::Event::Closed>()) {
                window.close();
                break;
            }

            if(const auto* resized = event->getIf<sf::Event::Resized>()) {
                float width = static_cast<float>(resized->size.x);
                float height = static_cast<float>(resized->size.y);
                if(width < 1.f || height < 1.f)
                    continue;
                updateLayout(width,height);
                window.setView(sf::View(sf::FloatRect({0.f,0.f},{width,height})));
                updateRouterLayout();
            }

            if(const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                if(mouse->button == sf::Mouse::Button::Left)
                    handleMouseClick(window.mapPixelToCoords(mouse->position));
            }

            if(const auto* typed = event->getIf<sf::Event::TextEntered>())
                handleTextInput(typed->unicode);

            if(const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if(key->code == sf::Keyboard::Key::Escape)
                    focusedInput = FocusedInput::None;
                if(key->code == sf::Keyboard::Key::Enter && focusedInput == FocusedInput::Message) {
                    focusedInput = FocusedInput::None;
                    sendMessage();
                }
            }
        }

        if(!window.isOpen())
            break;

        updatePacket(deltaTime);
        window.clear(WINDOW_BG);
        drawUI(window,font);
        drawConnections(window);
        drawShortestPath(window);
        drawRouters(window);
        drawRouterRing(window,sourceRouterId,SOURCE_COLOR);
        drawRouterRing(window,destinationRouterId,DESTINATION_COLOR,8.f);
        drawRouterRing(window,selectedRouterId,SELECTED_COLOR,12.f);
        drawPacket(window);
        drawRouterIDs(window,font);
        drawLinkCostTooltip(window,font);
        window.display();
    }

    UI_ASSETS.clear();
    return 0;
}

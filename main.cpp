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
#include <cmath>

extern "C" {
    #include "network.h"
    #include "graph.h"
    #include "dijkstra.h"
}

class Router : public sf::CircleShape {
private:
    bool working;
    sf::Vector2f worldPosition;
    float worldRadius;
public:
    Router(float x, float y, float radius, bool on)
        : sf::CircleShape(radius),
         working(on),
         worldPosition(x,y),
         worldRadius(radius)
    {
        setPointCount(80);
        if(working)
            setFillColor(sf::Color(91,165,155));
        else
            setFillColor(sf::Color(90,102,115));
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

std::unordered_map<std::string, sf::FloatRect> UI_RECT;
std::vector<Router> routers;

void updateRouterLayout() {
    const auto& canvas = UI_RECT.at("canvas");
    for(auto& router : routers)
        router.updateScreen(canvas);
}

const sf::Color WINDOW_BG(8,16,24);
const sf::Color CANVAS_BG(10,24,38);
const sf::Color SIDEBAR_BG(16,30,44);
const sf::Color FOOTER_BG(12,24,36);
const sf::Color TEXT_PRIMARY(230,236,242);
const sf::Color TEXT_MUTED(165,178,190);

const float ASSET_SCALE = 4.f;

struct Asset {
    std::unique_ptr<sf::Texture> texture;
    int insetPx = 0;
};

std::unordered_map<std::string, Asset> UI_ASSETS;

bool loadAsset(const std::string& name, int insetPx = 0) {
    auto texture = std::make_unique<sf::Texture>();
    if (!texture->loadFromFile("assets/ui/" + name + ".png")) {
        std::cerr << "Cannot load assets/ui/" << name << ".png\n";
        return false;
    }
    texture->setSmooth(true);
    UI_ASSETS.emplace(name, Asset{std::move(texture), insetPx});
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

    UI_RECT["add_router"] = {{left,contentHeight * 0.18f},{halfWidth,controlHeight}};
    UI_RECT["remove_router"] = {{left + halfWidth + gap,contentHeight * 0.18f},{halfWidth,controlHeight}};
    UI_RECT["toggle_router"] = {{left,contentHeight * 0.27f},{availableWidth,controlHeight}};

    UI_RECT["source"] = {{left,contentHeight * 0.45f},{halfWidth,controlHeight}};
    UI_RECT["destination"] = {{left + halfWidth + gap,contentHeight * 0.45f},{halfWidth,controlHeight}};
    UI_RECT["run_dijkstra"] = {{left,contentHeight * 0.54f},{availableWidth,controlHeight}};

    UI_RECT["message_input"] = {{left,contentHeight * 0.73f},{availableWidth,controlHeight}};
    UI_RECT["send_message"] = {{left,contentHeight * 0.82f},{availableWidth,controlHeight}};

    float logoWidth = std::min(190.f,availableWidth);
    float logoHeight = logoWidth * 24.f / 190.f;
    UI_RECT["logo"] = {{left,25.f},{logoWidth,logoHeight}};
}

void drawTile(sf::RenderWindow& window, const sf::Texture& texture,
              int sx, int sy, int sw, int sh,
              float dx, float dy, float dw, float dh) {
    if (sw <= 0 || sh <= 0 || dw <= 0.f || dh <= 0.f)
        return;

    sf::Sprite sprite(texture, sf::IntRect({sx,sy},{sw,sh}));
    sprite.setPosition({dx,dy});
    sprite.setScale({dw / static_cast<float>(sw),dh / static_cast<float>(sh)});
    window.draw(sprite);
}

void drawAsset(sf::RenderWindow& window, const std::string& name, const sf::FloatRect& rect) {
    const Asset& asset = UI_ASSETS.at(name);
    const sf::Texture& texture = *asset.texture;
    auto size = texture.getSize();
    int tw = static_cast<int>(size.x);
    int th = static_cast<int>(size.y);

    float x = rect.position.x;
    float y = rect.position.y;
    float w = rect.size.x;
    float h = rect.size.y;

    if (asset.insetPx == 0) {
        drawTile(window,texture,0,0,tw,th,x,y,w,h);
        return;
    }

    int sourceCorner = asset.insetPx;
    float corner = static_cast<float>(sourceCorner) / ASSET_SCALE;
    float left = std::min(corner,w / 2.f);
    float top = std::min(corner,h / 2.f);
    float right = left;
    float bottom = top;

    int sx[4] = {0,sourceCorner,tw-sourceCorner,tw};
    int sy[4] = {0,sourceCorner,th-sourceCorner,th};
    float dx[4] = {x,x+left,x+w-right,x+w};
    float dy[4] = {y,y+top,y+h-bottom,y+h};

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            drawTile(window,texture,
                     sx[col],sy[row],sx[col+1]-sx[col],sy[row+1]-sy[row],
                     dx[col],dy[row],dx[col+1]-dx[col],dy[row+1]-dy[row]);
        }
    }
}

void drawText(sf::RenderWindow& window, const sf::Font& font,
              const std::string& message, const sf::FloatRect& rect,
              unsigned int fontSize, const sf::Color& color) {
    sf::Text text(font,message,fontSize);
    text.setFillColor(color);
    auto bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                    bounds.position.y + bounds.size.y / 2.f});
    text.setPosition({rect.position.x + rect.size.x / 2.f,
                      rect.position.y + rect.size.y / 2.f});
    window.draw(text);
}

void drawLeftText(sf::RenderWindow& window, const sf::Font& font,
                  const std::string& message, float x, float y,
                  unsigned int fontSize, const sf::Color& color) {
    sf::Text text(font,message,fontSize);
    text.setFillColor(color);
    text.setPosition({x,y});
    window.draw(text);
}

void drawBackgrounds(sf::RenderWindow& window) {
    drawAsset(window,"main-panel",UI_RECT.at("canvas"));
    drawAsset(window,"sidebar-panel",UI_RECT.at("sidebar"));
    drawAsset(window,"status-panel",UI_RECT.at("footer"));
    drawAsset(window,"networklab-logo",UI_RECT.at("logo"));
}

void drawControls(sf::RenderWindow& window, const sf::Font& font, sf::Vector2f mouse) {
    const std::string buttons[] = {
        "add_router","remove_router","toggle_router",
        "run_dijkstra","send_message"
    };

    for (const std::string& name : buttons) {
        const auto& rect = UI_RECT.at(name);
        std::string assetName = "button";
        if (name == "run_dijkstra")
            assetName = "button-active";
        else if (rect.contains(mouse))
            assetName = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)
                        ? "button-pressed" : "button-hover";
        drawAsset(window,assetName,rect);
    }

    drawAsset(window,"input-field",UI_RECT.at("source"));
    drawAsset(window,"input-field",UI_RECT.at("destination"));
    drawAsset(window,"input-field",UI_RECT.at("message_input"));

    drawText(window,font,"Add",UI_RECT.at("add_router"),18,TEXT_PRIMARY);
    drawText(window,font,"Remove",UI_RECT.at("remove_router"),18,TEXT_PRIMARY);
    drawText(window,font,"Enable / Disable",UI_RECT.at("toggle_router"),17,TEXT_PRIMARY);
    drawText(window,font,"Source",UI_RECT.at("source"),17,TEXT_MUTED);
    drawText(window,font,"Destination",UI_RECT.at("destination"),17,TEXT_MUTED);
    drawText(window,font,"Run Dijkstra",UI_RECT.at("run_dijkstra"),18,TEXT_PRIMARY);
    drawText(window,font,"Message",UI_RECT.at("message_input"),17,TEXT_MUTED);
    drawText(window,font,"Send Message",UI_RECT.at("send_message"),18,TEXT_PRIMARY);
}

void drawUI(sf::RenderWindow& window, const sf::Font& font) {
    auto mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    drawBackgrounds(window);
    const auto& sidebar = UI_RECT.at("sidebar");
    float headingX = sidebar.position.x + sidebar.size.x * 0.08f;
    float headingHeight = sidebar.size.y;
    drawLeftText(window,font,"NETWORK",headingX,headingHeight * 0.18f - 33.f,15,TEXT_MUTED);
    drawLeftText(window,font,"ROUTING",headingX,headingHeight * 0.45f - 33.f,15,TEXT_MUTED);
    drawLeftText(window,font,"MESSAGE",headingX,headingHeight * 0.73f - 33.f,15,TEXT_MUTED);
    drawControls(window,font,mouse);

    const auto& footer = UI_RECT.at("footer");
    drawLeftText(window,font,"Network Simulator",24.f,footer.position.y+12.f,22,TEXT_PRIMARY);
    drawLeftText(window,font,"C engine  /  SFML visualizer",25.f,footer.position.y+44.f,14,TEXT_MUTED);
}


bool createRouters(){
    routers.clear();
    for(int i = 0 ; i < 20; i++){
        if(!createNewConnection()) {
            std::cerr << "FAIL: Network creation at " << i + 1 << '\n';
            return false;
        }
    }
    for(int j = 0 ; j < getNetworkCount(); j++){
        Network network = getNetwork(j + 1);
        routers.emplace_back(network.x,network.y,network.radius,network.working);
    }

    return getNetworkCount()==routers.size();
     
}

void drawRouters(sf::RenderWindow& window){
    for(const auto& router : routers)
        window.draw(router);
}

const sf::Color LINK_COLOR(70, 110, 140, 150);

void drawConnection(sf::RenderWindow& window,const sf::Vector2f& start,const sf::Vector2f& end,float thickness = 2.f) {
    float dx = end.x - start.x;
    float dy = end.y - start.y;

    float length = std::sqrt(dx * dx + dy * dy);
    float angleDeg = std::atan2(dy, dx) * 180.f / 3.14159265f;

    sf::RectangleShape line;
    line.setPosition(start);
    line.setSize({length, thickness});
    line.setFillColor(LINK_COLOR);
    
    line.setOrigin({0.f, thickness / 2.f});

    line.setRotation(sf::degrees(angleDeg));

    window.draw(line);
}

void getConnections(sf::RenderWindow& window) {
    int count = getNetworkCount();
    for(int i = 0; i < count; i++) {
        for(int j = i + 1; j < count; j++) {
            Link link = getLink(i + 1,j + 1);

            if(!isEmptyLink(link)) {
                sf::Vector2f start = routers[i].getPosition();
                sf::Vector2f end = routers[j].getPosition();

                drawConnection(window,start,end,2.f);
            }
        }
    }
}




int main() {
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode({1500,900}),"NetworkLab",sf::Style::Default,sf::State::Windowed,settings);
    window.setFramerateLimit(60);

    srand(42);
    initializeGraph();
    if(!createRouters())
        return 1;

    sf::Font font;
    if (!font.openFromFile("assets/fonts/Inter-Medium.ttf")) {
        std::cerr << "Cannot load assets/fonts/Inter-Medium.ttf\n";
        return 1;
    }
    if (!loadUIAssets())
        return 1;

    updateLayout(static_cast<float>(window.getSize().x),static_cast<float>(window.getSize().y));
    updateRouterLayout();

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                float width = static_cast<float>(resized->size.x);
                float height = static_cast<float>(resized->size.y);

                if (width < 1.f || height < 1.f)
                    continue;
                updateLayout(width,height);
                window.setView(sf::View(sf::FloatRect({0.f,0.f},{width,height})));
                updateRouterLayout();
            }
        }


        window.clear(WINDOW_BG);
        drawUI(window,font);

        getConnections(window);
       
        drawRouters(window);
        window.display();
    }
    UI_ASSETS.clear();

    return 0;
}

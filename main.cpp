#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <bits/stdc++.h>

class FlappyBird {
public:
    FlappyBird();
    ~FlappyBird();
    enum GameState { RUNNING, GAME_OVER, PAUSED };
    bool init();
    void run();

private:
    SDL_Window* gWindow;
    SDL_Renderer* gRenderer;
    SDL_Texture* gBackgroundTexture;
    SDL_Texture* gBirdTexture;
    SDL_Texture* gPipeUpTexture;
    SDL_Texture* gPipeDownTexture;
    SDL_Texture* exitTexture;
    SDL_Rect exitRect;
    SDL_Rect gBirdRect;
    int gravity;
    bool quit;

    struct PipePair {
        SDL_Rect upperRect;
        SDL_Rect lowerRect;
        bool passed;
    };

    std::vector<PipePair> pipes;

    TTF_Font* gFont;
    SDL_Texture* gTextTexture;

    SDL_Texture* playAgainTexture;
    SDL_Texture* quitTexture;

    SDL_Rect playAgainRect;
    SDL_Rect quitRect;

    int score;
    int highScore;

    GameState gameState;

    bool loadMedia();
    void close();
    void render();
    void handleEvents();
    void update();
    bool generatePipePair();
    bool checkCollision(const SDL_Rect& rect);
    bool loadFont();
    void renderGameOverText();
    void showGameOverScreen();
    void showPauseScreen();
    void updateScore();
    void renderScore();
};

FlappyBird::FlappyBird() : gWindow(nullptr), gRenderer(nullptr), gBackgroundTexture(nullptr), gBirdTexture(nullptr), gPipeUpTexture(nullptr), gPipeDownTexture(nullptr), gFont(nullptr), gTextTexture(nullptr), gravity(2.5), quit(false), gameState(PAUSED), score(0), highScore(0) {
    gBirdRect.x = 100;
    gBirdRect.y = 200;
    gBirdRect.w = 50;
    gBirdRect.h = 50;
}

FlappyBird::~FlappyBird() {
    close();
}

bool FlappyBird::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf initialization failed: " << TTF_GetError() << std::endl;
        return false;
    }

    gWindow = SDL_CreateWindow("Flappy Bird", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    if (gWindow == nullptr) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (gRenderer == nullptr) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
        return false;
    }

    if (!loadMedia() || !loadFont()) {
        std::cerr << "Media loading failed!" << std::endl;
        return false;
    }

    std::srand(std::time(0));

    if (!generatePipePair()) {
        std::cerr << "Failed to generate pipes!" << std::endl;
        return false;
    }

    score = 0;
    highScore = 0;

    return true;
}

bool FlappyBird::loadFont() {
    gFont = TTF_OpenFont("font/Font.ttf", 30);
    if (gFont == nullptr) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return false;
    }
    return true;
}

bool FlappyBird::loadMedia() {
    SDL_Surface* backgroundSurface = IMG_Load("img/bkground.png");
    if (backgroundSurface == nullptr) {
        std::cerr << "Failed to load background image: " << IMG_GetError() << std::endl;
        return false;
    }

    gBackgroundTexture = SDL_CreateTextureFromSurface(gRenderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);
    if (gBackgroundTexture == nullptr) {
        std::cerr << "Failed to create background texture: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_Surface* birdSurface = IMG_Load("img/bird.png");
    if (birdSurface == nullptr) {
        std::cerr << "Failed to load bird image: " << IMG_GetError() << std::endl;
        return false;
    }

    gBirdTexture = SDL_CreateTextureFromSurface(gRenderer, birdSurface);
    SDL_FreeSurface(birdSurface);
    if (gBirdTexture == nullptr) {
        std::cerr << "Failed to create bird texture: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_Surface* pipeUpSurface = IMG_Load("img/pipe_up.png");
    if (pipeUpSurface == nullptr) {
        std::cerr << "Failed to load pipe_up image: " << IMG_GetError() << std::endl;
        return false;
    }

    gPipeUpTexture = SDL_CreateTextureFromSurface(gRenderer, pipeUpSurface);
    SDL_FreeSurface(pipeUpSurface);
    if (gPipeUpTexture == nullptr) {
        std::cerr << "Failed to create pipe_up texture: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_Surface* pipeDownSurface = IMG_Load("img/pipe_down.png");
    if (pipeDownSurface == nullptr) {
        std::cerr << "Failed to load pipe_down image: " << IMG_GetError() << std::endl;
        return false;
    }

    gPipeDownTexture = SDL_CreateTextureFromSurface(gRenderer, pipeDownSurface);
    SDL_FreeSurface(pipeDownSurface);
    if (gPipeDownTexture == nullptr) {
        std::cerr << "Failed to create pipe_down texture: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void FlappyBird::close() {
    SDL_DestroyTexture(gBackgroundTexture);
    SDL_DestroyTexture(gBirdTexture);
    SDL_DestroyTexture(gPipeUpTexture);
    SDL_DestroyTexture(gPipeDownTexture);
    SDL_DestroyTexture(gTextTexture);
    SDL_DestroyTexture(playAgainTexture);
    SDL_DestroyTexture(quitTexture);

    TTF_CloseFont(gFont);

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void FlappyBird::render() {
    SDL_RenderClear(gRenderer);

    SDL_RenderCopy(gRenderer, gBackgroundTexture, nullptr, nullptr);

    SDL_RenderCopy(gRenderer, gBirdTexture, nullptr, &gBirdRect);

    for (const auto& pipePair : pipes) {
        SDL_RenderCopy(gRenderer, gPipeUpTexture, nullptr, &pipePair.upperRect);
        SDL_RenderCopy(gRenderer, gPipeDownTexture, nullptr, &pipePair.lowerRect);
    }

    if (gameState == GAME_OVER) {
        showGameOverScreen();
        renderGameOverText();
    } else if (gameState == PAUSED) {
        showPauseScreen();
    }

    renderScore();

    SDL_RenderPresent(gRenderer);
}

void FlappyBird::renderGameOverText() {
    SDL_Color redColor = {255, 0, 0, 255};
    TTF_Font* gameOverFont = TTF_OpenFont("font/Font.ttf", 50);
    if (gameOverFont == nullptr) {
        std::cerr << "Failed to load font for game over text: " << TTF_GetError() << std::endl;
        return;
    }
    SDL_Surface* gameOverSurface = TTF_RenderText_Solid(gameOverFont, "Game Over", redColor);
    if (gameOverSurface == nullptr) {
        std::cerr << "Failed to create surface for game over text: " << TTF_GetError() << std::endl;
        TTF_CloseFont(gameOverFont);
        return;
    }
    SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(gRenderer, gameOverSurface);
    if (gameOverTexture == nullptr) {
        std::cerr << "Failed to create texture for game over text: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(gameOverSurface);
        TTF_CloseFont(gameOverFont);
        return;
    }
    SDL_Rect gameOverRect = {320 - (gameOverSurface->w / 2), 100, gameOverSurface->w, gameOverSurface->h};
    SDL_RenderCopy(gRenderer, gameOverTexture, nullptr, &gameOverRect);
    SDL_FreeSurface(gameOverSurface);
    SDL_DestroyTexture(gameOverTexture);
    TTF_CloseFont(gameOverFont);
}

void FlappyBird::showPauseScreen() {
    SDL_Color whiteColor = { 255, 255, 255, 255 };
    SDL_Surface* pauseSurface = TTF_RenderText_Solid(gFont, "Press SPACE to Play", whiteColor);
    SDL_Texture* pauseTexture = SDL_CreateTextureFromSurface(gRenderer, pauseSurface);

    SDL_Rect pauseRect;
    pauseRect.x = 320 - (pauseSurface->w / 2);
    pauseRect.y = 240 - (pauseSurface->h / 2);
    pauseRect.w = pauseSurface->w;
    pauseRect.h = pauseSurface->h;

    SDL_RenderCopy(gRenderer, pauseTexture, nullptr, &pauseRect);
    SDL_FreeSurface(pauseSurface);
    SDL_DestroyTexture(pauseTexture);
}

void FlappyBird::showGameOverScreen() {
    SDL_Color whiteColor = { 255, 255, 255, 255 };
    SDL_Surface* playAgainSurface = TTF_RenderText_Solid(gFont, "Play Again", whiteColor);
    playAgainTexture = SDL_CreateTextureFromSurface(gRenderer, playAgainSurface);
    playAgainRect = { 320 - (playAgainSurface->w / 2), 300, playAgainSurface->w, playAgainSurface->h };
    SDL_FreeSurface(playAgainSurface);

    SDL_Surface* quitSurface = TTF_RenderText_Solid(gFont, "Quit", whiteColor);
    quitTexture = SDL_CreateTextureFromSurface(gRenderer, quitSurface);
    quitRect = { 320 - (quitSurface->w / 2), 350, quitSurface->w, quitSurface->h };
    SDL_FreeSurface(quitSurface);

    SDL_RenderCopy(gRenderer, playAgainTexture, nullptr, &playAgainRect);
    SDL_RenderCopy(gRenderer, quitTexture, nullptr, &quitRect);
}

void FlappyBird::renderScore() {
    SDL_Color whiteColor = { 255, 255, 255, 255 };
    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(gFont, scoreText.c_str(), whiteColor);
    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(gRenderer, scoreSurface);

    SDL_Rect scoreRect;
    scoreRect.x = 10;
    scoreRect.y = 10;
    scoreRect.w = scoreSurface->w;
    scoreRect.h = scoreSurface->h;

    SDL_RenderCopy(gRenderer, scoreTexture, nullptr, &scoreRect);
    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(scoreTexture);
}

void FlappyBird::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_SPACE) {
                if (gameState == PAUSED) {
                    gameState = RUNNING;
                } else if (gameState == RUNNING) {
                    gBirdRect.y -= 50;
                } else if (gameState == GAME_OVER) {
                    gameState = PAUSED;
                    gBirdRect.y = 200;
                    pipes.clear();
                    generatePipePair();
                    score = 0;
                }
            }
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;
            if (gameState == GAME_OVER) {
                if (mouseX >= playAgainRect.x && mouseX <= playAgainRect.x + playAgainRect.w && mouseY >= playAgainRect.y && mouseY <= playAgainRect.y + playAgainRect.h) {
                    gameState = PAUSED;
                    gBirdRect.y = 200;
                    pipes.clear();
                    generatePipePair();
                    score = 0;
                } else if (mouseX >= quitRect.x && mouseX <= quitRect.x + quitRect.w && mouseY >= quitRect.y && mouseY <= quitRect.y + quitRect.h) {
                    quit = true;
                }
            }
        }
    }
}

void FlappyBird::update() {
    if (gameState == RUNNING) {
        gBirdRect.y += gravity;
        for (auto& pipePair : pipes) {
            pipePair.upperRect.x -= 5;
            pipePair.lowerRect.x -= 5;

            if (!pipePair.passed && pipePair.upperRect.x < gBirdRect.x) {
                pipePair.passed = true;
                score++;
            }

            if (pipePair.upperRect.x + pipePair.upperRect.w < 0) {
                pipes.erase(pipes.begin());
                generatePipePair();
            }
        }

        if (checkCollision(gBirdRect)) {
            gameState = GAME_OVER;
            if (score > highScore) {
                highScore = score;
            }
        }
    }
}

bool FlappyBird::generatePipePair() {
    PipePair newPair;
    newPair.upperRect = { 640, 0, 50, 200 };
    newPair.lowerRect = { 640, 300, 50, 200 };
    newPair.passed = false;
    pipes.push_back(newPair);
    return true;
}

bool FlappyBird::checkCollision(const SDL_Rect& rect) {
    for (const auto& pipePair : pipes) {
        if (SDL_HasIntersection(&rect, &pipePair.upperRect) || SDL_HasIntersection(&rect, &pipePair.lowerRect)) {
            return true;
        }
    }

    if (rect.y <= 0 || rect.y + rect.h >= 480) {
        return true;
    }

    return false;
}

void FlappyBird::run() {
    while (!quit) {
        handleEvents();
        update();
        render();
        SDL_Delay(16);
    }
}

int main(int argc, char* args[]) {
    FlappyBird game;
    if (!game.init()) {
        std::cerr << "Failed to initialize!" << std::endl;
        return -1;
    }
    game.run();
    return 0;
}

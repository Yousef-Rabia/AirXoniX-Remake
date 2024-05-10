#pragma once

#include <glm/vec2.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <string>
#include <unordered_map>
#include <type_traits>
#include <json/json.hpp>

#include "input/keyboard.hpp"
#include "input/mouse.hpp"
#include "sound/sound.hpp"

// constants
#define ARENA_LENGTH 19
#define INITIAL_PLAYER_POSITION glm::vec3(0, 3, 19)
#define INITIAL_CAMERA_POSITION glm::vec3(0, 18, 25)
#define INITIAL_MINE_POSITION glm::vec3(0, 1.5, -19)
#define AUTO_MOVEMENT_SPEED 20
#define INITIAL_LIVES 5
#define FINISH_PERCENTAGE 90

// collision hit-boxes
#define ENEMY_ENEMY_HITBOX 5
#define ENEMY_PLAYER_HITBOX 1
#define ENEMY_LINE_HITBOX 2.5
#define BALL_CUBE_HITBOX 3
#define MINE_CUBE_HITBOX 1

namespace our {

    // This struct handles window attributes: (title, size, isFullscreen).
    struct WindowConfiguration {
        std::string title;
        glm::i16vec2 size;
        bool isFullscreen;
    };

    class Application; // Forward declaration

    // This is the base class for all states
    // The application will be responsible for managing all scene functionality by calling the "on*" functions.
    class State {
        // Each scene will have a pointer to the application that owns it
        Application* application;
        friend Application;
    public:
        virtual void onInitialize(){}                   // Called once before the game loop.
        virtual void onImmediateGui(){}                 // Called every frame to draw the Immediate GUI (if any).
        virtual void onDraw(double deltaTime){}         // Called every frame in the game loop passing the time taken to draw the frame "Delta time".
        virtual void onDestroy(){}                      // Called once after the game loop ends for house cleaning.


        // Override these functions to get mouse and keyboard event.
        virtual void onKeyEvent(int key, int scancode, int action, int mods){}      
        virtual void onCursorMoveEvent(double x, double y){}
        virtual void onCursorEnterEvent(int entered){}
        virtual void onMouseButtonEvent(int button, int action, int mods){}
        virtual void onScrollEvent(double x_offset, double y_offset){}

        //Returns a pointer
        Application* getApp() { return application; }
    };

    // This class act as base class for all the Applications covered in the examples.
    // It offers the functionalities needed by all the examples.
    class Application {
    protected:
        GLFWwindow * window = nullptr;      // Pointer to the window created by GLFW using "glfwCreateWindow()".
        
        Keyboard keyboard;                  // Instance of "our" keyboard class that handles keyboard functionalities.
        Mouse mouse;                        // Instance of "our" mouse class that handles mouse functionalities.

        nlohmann::json app_config;           // A Json file that contains all application configuration

        std::unordered_map<std::string, State*> states;   // This will store all the states that the application can run
        State * currentState = nullptr;         // This will store the current scene that is being run
        State * nextState = nullptr;            // If it is requested to go to another scene, this will contain a pointer to that scene

        
        // Virtual functions to be overrode and change the default behaviour of the application
        // according to the example needs.
        virtual void configureOpenGL();                             // This function sets OpenGL Window Hints in GLFW.
        virtual WindowConfiguration getWindowConfiguration();       // Returns the WindowConfiguration current struct instance.
        virtual void setupCallbacks();                              // Sets-up the window callback functions from GLFW to our (Mouse/Keyboard) classes.

    public:
        // Game variables
        bool paused = false; // pauses the entire game
        int coveredArea = 0; // when it reaches 100% you win the game
        int lives = INITIAL_LIVES; // when they reach zero you lose
        Sound soundPlayer;

        // Create an application with following configuration
        Application(const nlohmann::json& app_config) : app_config(app_config) {}
        // On destruction, delete all the states
        ~Application(){ for (auto &it : states) delete it.second; }

        // This is the main class function that run the whole application (Initialize, Game loop, House cleaning).
        int run(int run_for_frames = 0);

        // Functions to show gui elements after certain events
        void status(ImFont *pFont) const;
        void win(ImFont *pFont);
        void lose(ImFont *pFont);

        // Register a state for use by the application
        // The state is uniquely identified by its name
        // If the name is already used, the old name owner is deleted and the new state takes its place
        template<typename T>
        void registerState(std::string name){
            static_assert(std::is_base_of<State, T>::value, "T must derive from our::State");
            auto it = states.find(name);
            if(it != states.end()){
                delete it->second;
            }
            State* scene = new T();
            scene->application = this;
            states[name] = scene;
        }

        // Tells the application to change its current state
        // The change will not be applied until the current frame ends
        void changeState(std::string name){
            auto it = states.find(name);
            if(it != states.end()){
                nextState = it->second;
            }
        }

        // Closes the Application
        void close(){
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Class Getters.
        GLFWwindow* getWindow(){ return window; }
        [[nodiscard]] const GLFWwindow* getWindow() const { return window; }
        Keyboard& getKeyboard() { return keyboard; }
        [[nodiscard]] const Keyboard& getKeyboard() const { return keyboard; }
        Mouse& getMouse() { return mouse; }
        [[nodiscard]] const Mouse& getMouse() const { return mouse; }

        [[nodiscard]] const nlohmann::json& getConfig() const { return app_config; }

        // Get the size of the frame buffer of the window in pixels.
        glm::ivec2 getFrameBufferSize() {
            glm::ivec2 size;
            glfwGetFramebufferSize(window, &(size.x), &(size.y));
            return size;
        }

        // Get the window size. In most cases, it is equal to the frame buffer size.
        // But on some platforms, the framebuffer size may be different from the window size.
        glm::ivec2 getWindowSize() {
            glm::ivec2 size;
            glfwGetWindowSize(window, &(size.x), &(size.y));
            return size;
        }
    };
}

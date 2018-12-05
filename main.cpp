#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <ctime>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void generateObstacles(int nObstacles, vector<glm::mat4>& obstacles);
void drawObstacles(vector<glm::mat4>& obstacles, Model rock, Shader ourShader);
void moveObstacles(vector<glm::mat4>& obstacles, int turn);
void destroyObstacles(float z, vector<glm::mat4>& obstacles, int& points);

//global variables
float z ;
float x ;
float y ;
int timeTotal  = 5; //velocidade de deslocamento dos objetos
int depthMax  = 15; //deslocamento total no eixo z que os objetos poderao estar
int depthMin =  10;
int xyMax  = 4;
int points;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader(FileSystem::getPath("resources/cg_ufpel.vs").c_str(), FileSystem::getPath("resources/cg_ufpel.fs").c_str());

    // load models
    // -----------
    Model ourModel(FileSystem::getPath("resources/objects/nanosuit/nanosuit.obj"));
    //Model ourModel(FileSystem::getPath("resources/objects/z4e9hljoq1hc-tie_fighter/TIE-fighter.obj"));
    Model rock(FileSystem::getPath("resources/objects/rock/rock.obj"));

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);



    //#include <ctime>
    srand(time(NULL));  //para gerar objetos na tela
    //cout << (rand() %10) + 1;

    float turn = 0;
    int timeTotal = 5; //velocidade de deslocamento dos objetos
    int depthMax = 25; //deslocamento total no eixo z que os objetos poderao estar
    int depthMin = 15;
    int xyMax = 4;

    float z ;
    float x ;
    float y ;

    vector<glm::mat4> obstacles;

    int difficulty = 1;
    bool generate = true;
    bool flag = false;
    int generateFlag = 1;
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)){
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        //turn = (currentFrame*z)/timeTotal;
        turn = 0.05;
        /*
        currentFrame   ----   ?
        timeTotal     -----   depthTotal
        */

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 model;
        model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        //game preparation
        if(generate){ generateObstacles(difficulty, obstacles); generate=false; }
        drawObstacles(obstacles, rock, ourShader);

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) { flag = !flag; }

        if (flag) {
            //the game begins
            //translate de obstacles (move our starship)
            for(int i=0; i < (int)obstacles.size(); ++i) {
                float translate =  obstacles[i][3][2] + turn;
                obstacles[i][3][2] = translate;
           }
           int curPoints = points;
           //check when obstacles get out the plan
           destroyObstacles(model[3][2], obstacles, points);
           //increase the difficulty and generate more obstacles
           if(points > curPoints) { ++generateFlag; generate = true;} //check if the obstacles are destroyed
           if(generateFlag%7 == 0) { ++difficulty; ++generateFlag; }
           if(obstacles.size() == 7) { difficulty = 1; }
           //see the colisions
        }
        //cout << "number of obstacles: " << obstacles.size() << endl;

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


void generateObstacles(int nObstacles,vector<glm::mat4>& obstacles){
  for(int i=0;i<nObstacles;++i) {
        glm::mat4 matrixGenerator;
        int signalX = rand() % 2;
        int signalY = rand() % 2;
        //cout << "signalX: " << signalX << endl;
        z = (((5 + (rand() % ( 15 - 5 + 1 )))) * (-1));
        x = rand() % xyMax;
        y = rand()% xyMax;

        if(signalX==1) { x *= (-1); }
        if(signalY==1) { y *= (-1); }

        matrixGenerator = glm::translate(matrixGenerator, glm::vec3(x, y, z));
        matrixGenerator = glm::scale(matrixGenerator, glm::vec3(0.5f, 0.5f, 0.5f));

        obstacles.push_back( matrixGenerator );
  }
}


void drawObstacles(vector<glm::mat4>& obstacles, Model rock, Shader ourShader){
  for(int i=0; i < (int)obstacles.size(); ++i) {
      ourShader.setMat4("model", obstacles[i]);
      rock.Draw(ourShader);
  }
}

void moveObstacles(vector<glm::mat4>& obstacles, int turn){
  for(int i=0; i < (int)obstacles.size(); ++i) {
      float translate =  obstacles[i][3][2] + turn;
      obstacles[i][3][2] = translate;
  }
}

void destroyObstacles(float z, vector<glm::mat4>& obstacles, int& points){
  for(int i=0; i < (int)obstacles.size(); ++i) {
      if(obstacles[i][3][2] > z) { obstacles.erase(obstacles.begin() + i); ++points;}
  }
}

//!Includes
#include <GL/glew.h>
#include <GL/glut.h>
#include <Shader.h>
#include <Vector.h>
#include <Matrix.h>
#include <Mesh.h>
#include <Texture.h>
#include <SphericalCameraManipulator.h>
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>

//!Field terrain
#define FieldHeight 10
#define FieldWidth 10
int Field[FieldHeight][FieldWidth];

//!Function Prototypes
bool initGL(int argc, char** argv);

void initShader();
void display(void);
void initGeometry();
void drawGeometry();
void keyboard(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void handleKeys();
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void Timer(int value);
void initTexture(std::string filename, GLuint & textureID);
void render2dText(std::string text, float r, float g, float b, float x, float y);

int initmap();
int chooselevel();

//Global Variables
GLuint shaderProgramID;
GLuint vertexPositionAttribute;
GLuint vertexNormalAttribute;
GLuint vertexTexcoordAttribute;
GLuint TextureMapUniformLocation;

//Material Properties
GLuint LightPositionUniformLocation;                // Light Position Uniform   
GLuint AmbientUniformLocation;
GLuint SpecularUniformLocation;
GLuint SpecularPowerUniformLocation;

Vector3f lightPosition= Vector3f(15.0,15.0,5.0);   // Light Position 
Vector3f ambient    = Vector3f(0.2,0.15,0.1);
Vector3f specular   = Vector3f(0.1,0.5,1.0);
float specularPower = 10.0;


//Game boundary definitions
#define MAX_ACCEL 0.05

//Coin rotation
int coin_rotation_angle = 0;

//Tank
int tankmove = 0;
int tankturn = 0;
int turretturn = 0;

float tank_scale = 0.15;
float tank_x = 0;
float tank_y = 0;
float ball_scale = 0.1;
float ball_x = 0;
float ball_y = 0;

int shoot = 0;
int projectiletime = 0;
//Time

//Zoom
int zoom = 2;

//Game
int gamestate = 0;
int totalcoin = 0;
int level = 1;
int timeremaining = 1;
int currentcoin = 0;
int camera = 1;
int menu = 0;

float t = 0;

//Viewing
SphericalCameraManipulator cameraManip;
Matrix4x4 ModelViewMatrix;
GLuint MVMatrixUniformLocation;
Matrix4x4 ProjectionMatrix;
GLuint ProjectionUniformLocation;

//Texture
GLuint texture_ball;
GLuint texture_coin;
GLuint texture_Crate;
GLuint texture_hamvee;

//Mesh
Mesh back_wheel;
Mesh ball;
Mesh chassis;
Mesh coin;
Mesh cube;
Mesh front_wheel;
Mesh turret;

//! Screen size
int screenWidth   	        = 720;
int screenHeight   	        = 720;

//! Array of key states
bool keyStates[256];


//! Main Program Entry
int main(int argc, char** argv)
{	
	//init OpenGL
	if(!initGL(argc, argv))
		return -1;

    //Init Key States to false;    
    for(int i = 0 ; i < 256; i++)
        keyStates[i] = false;
    
	//Init map
	initmap();
	
	//Init OpenGL Shader
    initShader();
	
	//Init Mesh Geometry
	back_wheel.loadOBJ("../models/back_wheel.obj");
	ball.loadOBJ("../models/ball.obj");
	chassis.loadOBJ("../models/chassis.obj");
    coin.loadOBJ("../models/coin.obj");
	cube.loadOBJ("../models/cube.obj");
	front_wheel.loadOBJ("../models/front_wheel.obj");
	turret.loadOBJ("../models/turret.obj");

    //Init Camera Manipultor
	cameraManip.setPanTiltRadius(0.f,0.f,2.f);
	cameraManip.setFocus(Vector3f(tank_x, 1.0f, tank_y));

	//Init texture
	initTexture("../models/ball.bmp", texture_ball);
	initTexture("../models/coin.bmp", texture_coin);
	initTexture("../models/Crate.bmp", texture_Crate);
	initTexture("../models/hamvee.bmp", texture_hamvee);

    //Set up your program
    
    //Enter main loop
    glutMainLoop();

    //Delete shader program
	glDeleteProgram(shaderProgramID);

    return 0;
}

//! Function to Initlise OpenGL
bool initGL(int argc, char** argv)
{
	//Init GLUT
    glutInit(&argc, argv);
    
	//Set Display Mode
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);

	//Set Window Size
    glutInitWindowSize(screenWidth, screenHeight);
    
    // Window Position
    glutInitWindowPosition(200, 200);

	//Create Window
    glutCreateWindow("Tank Assignment");
    
    // Init GLEW
	if (glewInit() != GLEW_OK) 
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return false;
	}
	
	//Set Display function
    glutDisplayFunc(display);
	glClearColor(0.7, 0.72, 0.85, 1.0);
	//Set Keyboard Interaction Functions
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyUp); 

	//Set Mouse Interaction Functions
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(motion);
	glutMotionFunc(motion);



	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
    //Start start timer function after 100 milliseconds
    glutTimerFunc(100,Timer, 0);

	return true;
}

//Init Shader
void initShader()
{
	//Create shader
    shaderProgramID = Shader::LoadFromFile("shader.vert","shader.frag");
    
    // Get a handle for our vertex position buffer
	vertexPositionAttribute = glGetAttribLocation(shaderProgramID, "aVertexPosition");
	vertexNormalAttribute = glGetAttribLocation(shaderProgramID, "aVertexNormal");
	vertexTexcoordAttribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");


    //!
	MVMatrixUniformLocation = glGetUniformLocation(shaderProgramID, "MVMatrix_uniform"); 
	ProjectionUniformLocation = glGetUniformLocation(shaderProgramID, "ProjMatrix_uniform"); 

LightPositionUniformLocation    = glGetUniformLocation(shaderProgramID, "LightPosition_uniform"); 
	AmbientUniformLocation          = glGetUniformLocation(shaderProgramID, "Ambient_uniform"); 
	SpecularUniformLocation         = glGetUniformLocation(shaderProgramID, "Specular_uniform"); 
	SpecularPowerUniformLocation    = glGetUniformLocation(shaderProgramID, "SpecularPower_uniform");

	TextureMapUniformLocation = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");
}

void initTexture(std::string filename, GLuint & textureID)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    //Get texture Data
    int width, height;
    char * data;
    Texture::LoadBMP(filename, width, height, data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    //Cleanup data as copied to GPU
    delete[] data;
}

//! Display Loop
void display(void)
{
    //Handle keys
    handleKeys();

	//Set Viewport
	glViewport(0,0,screenWidth, screenHeight);

	// Clear the screen
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	//Use shader
	glUseProgram(shaderProgramID);

	//Apply Camera Manipluator to Set Model View Matrix on GPU
	ModelViewMatrix.toIdentity();
    //Projection Matrix - Perspective Projection
    ProjectionMatrix.perspective(90, 1.0, 0.0001, 100.0);

	//Set Projection Matrix
    glUniformMatrix4fv(	
		ProjectionUniformLocation,  //Uniform location
		1,							//Number of Uniforms
		false,						//Transpose Matrix
		ProjectionMatrix.getPtr());	//Pointer to ModelViewMatrixValues



    //Draw your scene
/*
	int tank_x1;
	int tank_y1;

	tank_x1 = static_cast<int>(tank_x);
	tank_y1 = static_cast<int>(tank_y);
	
	//Falling off map, empty crates and beyond 10x10 map boundaries
	if (Field[tank_x1][tank_y1] == 0 || tank_x < -1 || tank_x > 9 || tank_y < -1 || tank_y > 9)
	{
		tank_scale = 0;
		gamestate = -1;
	}

	//Collecting coins
	if (shoot == 0) 	//Driving over the coins
	{
		

		if((tank_x - tank_x1) > 0.1)
		{
			tank_x1++;
		}
	
		if((tank_y - tank_y1) > 0.1)
		{
			tank_y1++;
		}

		if (Field[tank_x1][tank_y1] == 2)
		{
			Field[tank_x1][tank_y1] = 1;
			currentcoin++;
		}
	}
*/
	
	int tank_y1;
    int tank_x1;
    int coin_y;
    int coin_x;

    if (shoot == 0)
    {
        tank_y1 = (tank_y/2);
        tank_x1 = (tank_x/2);

        if ((tank_y - trunc(tank_y/2)*2) > 1) 
		{
			tank_y1++;
		}

        if ((tank_x - trunc(tank_x/2)*2) > 1) 
		{
			tank_x1++;
		}

        if (Field[tank_x1][tank_y1] == 2)
            {
				Field[tank_x1][tank_y1] = 1;
                currentcoin++;
                totalcoin++;
             }
    }

    if (shoot == 1)
    {
        coin_y = (ball_y/2);
        coin_x = (ball_x/2);

        if ((ball_y - trunc(ball_y/2)*2) > 1) 
		{
			coin_y++;
		}

        if ((ball_x - trunc(ball_x/2)*2) > 1) 
		{
			coin_x++;
		}

        if (Field[coin_x][coin_y] == 2)
        {
            Field[coin_x][coin_y] = 1;
            currentcoin++;
            totalcoin++;
        }
    }

	//Falling off map, empty crates and beyond map boundaries
	if (Field[tank_x1][tank_y1] == 0 || tank_x < -1 || tank_x > 19 || tank_y < -1 || tank_y > 19)
	{
		tank_scale = 0;
		ball_scale = 0;
		gamestate = -1;
	}

	//Count total coins
	totalcoin = 0;

	for (int y=0; y<FieldHeight; y++) 
	{
		for (int x=0; x<FieldWidth; x++)
		{
			if (Field[x][y] == 2)
				{
					totalcoin++;
				}
		}
	}

	//Rotate the coins
    coin_rotation_angle++;

	//Tank shoot
	if (shoot == 1 && projectiletime <= 100)
	{
		ball_x = ball_x + MAX_ACCEL * sin(turretturn * M_PI / 180);
		ball_y = ball_y + MAX_ACCEL * cos(turretturn * M_PI / 180);

		projectiletime++;
	}
	
	if (projectiletime == 100)
	{
		ball_x = tank_x;
		ball_y = tank_y;

		projectiletime = 1;
		shoot = 0;
	}


	

	//Function to create cube crate terrain field
    for (int y=0; y<FieldHeight; y++) //2D arrays are accessed column first so iterate y then x to allow code to run faster.
	{
		for (int x=0; x<FieldWidth; x++)
		{
			if (Field[x][y] == 1 or Field[x][y] == 2)  	//Crate
			{
				
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture_Crate);
				glUniform1i(TextureMapUniformLocation, 0);

				glUniform3f(LightPositionUniformLocation, lightPosition.x,lightPosition.y,lightPosition.z);
    glUniform4f(AmbientUniformLocation, ambient.x, ambient.y, ambient.z, 1.0);
    glUniform4f(SpecularUniformLocation, specular.x, specular.y, specular.z, 1.0);
    glUniform1f(SpecularPowerUniformLocation, specularPower);

				Matrix4x4 Cr = cameraManip.apply(ModelViewMatrix);
				//Cr.scale(5.0, 5.0, 5.0);
				Cr.translate(x*2, 0.0, y*2);
				glUniformMatrix4fv(	
					MVMatrixUniformLocation,  	//Uniform location
					1,					        //Number of Uniforms
					false,				        //Transpose Matrix
	    			Cr.getPtr());	        //Pointer to Matrix Values

				cube.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
			}

			if (Field[x][y] == 2) 	//Coin
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture_coin);
				glUniform1i(TextureMapUniformLocation, 0);

				Matrix4x4 c = cameraManip.apply(ModelViewMatrix);
				
				c.scale(0.5, 0.5, 0.5);
				c.translate(x*4, 3.0, y*4);
				c.rotate(coin_rotation_angle, 0.0 , 1.0 , 0.0);
				glUniformMatrix4fv(	
					MVMatrixUniformLocation,  	//Uniform location
					1,					        //Number of Uniforms
					false,				        //Transpose Matrix
					c.getPtr());	        //Pointer to Matrix Values

				coin.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
			}

		}
	}

	//Back wheel

	Matrix4x4 bw = cameraManip.apply(ModelViewMatrix);

	bw.translate(tank_x, 0.95, tank_y);
	bw.scale(tank_scale, tank_scale, tank_scale);

	glUniformMatrix4fv(	
		MVMatrixUniformLocation,  	//Uniform location
		1,					        //Number of Uniforms
		false,				        //Transpose Matrix
	    bw.getPtr());	        //Pointer to Matrix Values

	back_wheel.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_hamvee);
	glUniform1i(TextureMapUniformLocation, 0);

	//Chassis
	Matrix4x4 ch = cameraManip.apply(ModelViewMatrix);

	ch.translate(tank_x, 0.95, tank_y);
	ch.scale(tank_scale, tank_scale, tank_scale);

	glUniformMatrix4fv(	
		MVMatrixUniformLocation,  	//Uniform location
		1,					        //Number of Uniforms
		false,				        //Transpose Matrix
	    ch.getPtr());	        //Pointer to Matrix Values

	chassis.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_hamvee);
	glUniform1i(TextureMapUniformLocation, 0);

	//Front wheel
	Matrix4x4 fw = cameraManip.apply(ModelViewMatrix);

	fw.translate(tank_x, 0.95, tank_y);
	fw.scale(tank_scale, tank_scale, tank_scale);

	glUniformMatrix4fv(	
		MVMatrixUniformLocation,  	//Uniform location
		1,					        //Number of Uniforms
		false,				        //Transpose Matrix
	    fw.getPtr());	        //Pointer to Matrix Values

	front_wheel.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_hamvee);
	glUniform1i(TextureMapUniformLocation, 0);
	
	//Turret
	Matrix4x4 tur = cameraManip.apply(ModelViewMatrix);

	tur.translate(tank_x, 0.95, tank_y);
	tur.scale(tank_scale, tank_scale, tank_scale);

	glUniformMatrix4fv(	
		MVMatrixUniformLocation,  	//Uniform location
		1,					        //Number of Uniforms
		false,				        //Transpose Matrix
	    tur.getPtr());	        //Pointer to Matrix Values

	turret.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_hamvee);
	glUniform1i(TextureMapUniformLocation, 0);
	
	//Ball
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_ball);
	glUniform1i(TextureMapUniformLocation, 0);

	Matrix4x4 b = cameraManip.apply(ModelViewMatrix);

	b.translate(ball_x, 1.22, ball_y);
	b.scale(ball_scale, ball_scale, ball_scale);

	glUniformMatrix4fv(	
		MVMatrixUniformLocation,  	//Uniform location
		1,					        //Number of Uniforms
		false,				        //Transpose Matrix
	    b.getPtr());	        //Pointer to Matrix Values

	ball.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);





	//Camera view options
	if (camera == 1)
	{
		//Camera focus set on tank, allows for zoom
		cameraManip.setFocus(Vector3f(tank_x, 1.0f, tank_y));
		cameraManip.setPanTiltRadius((float)(tankturn * 0.0175), -0.95f, (float)(zoom));
	}

	if (camera == -1)
	{
		//Camera focus on mouse
		cameraManip.setFocus(Vector3f(tank_x, 1.8f, tank_y));
		cameraManip.setPanTiltRadius((float)(turretturn * 0.0175), -1.00f, 0.07f);
	}

	if (camera == 0)
	{
		//Camera focus set on tank, allows for zoom, allows for mouse view control in mouse handle
		cameraManip.setFocus(Vector3f(tank_x, 1.0f, tank_y));
		cameraManip.setPanTiltRadius((float)(tankturn * 0.0175), -0.95f, (float)(zoom));
	}
	
	glUseProgram(0);

	//HUD

	//Menu
	if (menu == 1)
	{
        render2dText("Turret Rotation(e,q)", 1.0, 1.0, 1.0, -1.0, -0.80);
        render2dText("Tank Movement(wasd)", 1.0, 1.0, 1.0, -1.0, -0.85);
        render2dText("Camera Views(iop)", 1.0, 1.0, 1.0, -1.0, -0.90);
        render2dText("Choose Level(1-5)", 1.0, 1.0, 1.0, -1.0, -0.95);
        render2dText("Zoom In(=) Out(-)", 1.0, 1.0, 1.0, -1.0, -1.00);
	}

	timeremaining = int(90 - t);

	std::string t1 = "Level: ";
	std::string t2 = "Time remaining: ";
	std::string t3 = "Token(s) collected: ";
	std::string t4 = "/";

	std::string textlevel;
	std::string time;
	std::string tokens;

	std::stringstream sstm;
	std::stringstream sstm1;
	std::stringstream sstm2;

	sstm << t1 << level;
	sstm1 << t2 << timeremaining;
	sstm2 << t3 << currentcoin << t4 << totalcoin+currentcoin;

	textlevel = sstm.str();
	time = sstm1.str();
	tokens = sstm2.str();

	render2dText(textlevel, 1.0, 1.0, 1.0, -1.0, 0.95);
	render2dText(time, 1.0, 1.0, 1.0, -1.0, 0.90);
	render2dText(tokens, 1.0, 1.0, 1.0, -1.0, 0.85);
	render2dText("Press m for menu, n to close menu", 1.0, 1.0, 1.0, -1.0, 0.80);

	//End screen
	if (currentcoin == totalcoin+currentcoin) 	//Level complete
	{
		glClearColor(0.0, 1.0, 0.0, 0.0);
		render2dText("Congratulations you won! Try a new level (1-5) or quit Esc", 1.0, 1.0, 1.0, -0.65, 0.2);
	}

	if (gamestate == -1 || timeremaining <= 0) 	//Level fail
	{
		glClearColor(1.0, 0.0, 0.0, 0.0);
		render2dText("You lost, press Esc to try again" , 1.0, 1.0, 1.0, -0.3, 0.2);
		tank_scale = 0;
		ball_scale = 0;
	}

	glutPostRedisplay();

    //Swap Buffers and post redisplay
	glutSwapBuffers();
}



//! Keyboard Interaction
void keyboard(unsigned char key, int x, int y)
{
	//Quits program when esc is pressed
	if (key == 27)	//esc key code
	{
		exit(0);
	}
    
	else if(key == 'm')
	{
		std::cout << "m input" << std::endl;
		menu = 1;
	}

	else if(key == 'n')
	{
		std::cout << "n input" << std::endl;
		menu = 0;
	}

    else if(key == 'w')
	{
		std::cout << "w input" << std::endl;
	}

    else if(key == 's')
	{
		std::cout << "s input" << std::endl;
	}

    else if(key == 'a')
	{
		std::cout << "a input" << std::endl;
	}

    else if(key == 'd')
	{
		std::cout << "d input" << std::endl;
	}

    else if(key == 'e')
	{
		std::cout << "e input" << std::endl;
	}

    else if(key == 'q')
	{
		std::cout << "q input" << std::endl;
	}

	else if(key == ' ')
	{
		std::cout << "space bar input" << std::endl;
		shoot = 1;
	}

	else if(key == 'i')
	{
		std::cout << "i input" << std::endl;
		camera = 1;
	}

	else if(key == 'o')
	{
		std::cout << "o input" << std::endl;
		camera = -1;
	}

	else if(key == 'p')
	{
		std::cout << "p input" << std::endl;
		camera = 0;
	}

	else if(key == '1')
	{
		std::cout << "1 input" << std::endl;
		level = 2;
		chooselevel();
	}

	else if(key == '2')
	{
		std::cout << "2 input" << std::endl;
		level = 3;
		chooselevel();
	}

	else if(key == '3')
	{
		std::cout << "3 input" << std::endl;
		level = 4;
		chooselevel();
	}

	else if(key == '4')
	{
		std::cout << "4 input" << std::endl;
		level = 5;
		chooselevel();
	}

	else if(key == '5')
	{
		std::cout << "5 input" << std::endl;
		level = 6;
		chooselevel();
	}
	
    //Set key status
    keyStates[key] = true;
}

//! Handle key up situation
void keyUp(unsigned char key, int x, int y)
{
    keyStates[key] = false;
}


//! Handle Keys
void handleKeys()
{
    //keys should be handled here
	if(keyStates['w'])
    {
        tank_x = tank_x + MAX_ACCEL * sin(tankturn * M_PI / 180);
		tank_y = tank_y + MAX_ACCEL * cos(tankturn * M_PI / 180);

		ball_x = ball_x + MAX_ACCEL * sin(tankturn * M_PI / 180);
		ball_y = ball_y + MAX_ACCEL * cos(tankturn * M_PI / 180);
    }

	if(keyStates['a'])
    {
        tankturn++; 	//move left
    }

	if(keyStates['s'])
    {
        tank_x = tank_x - (MAX_ACCEL - 0.02) * sin(tankturn * M_PI / 180);
		tank_y = tank_y - (MAX_ACCEL - 0.02) * cos(tankturn * M_PI / 180);

		ball_x = ball_x - (MAX_ACCEL - 0.02) * sin(tankturn * M_PI / 180);
		ball_y = ball_y - (MAX_ACCEL - 0.02) * cos(tankturn * M_PI / 180);
    }

	if(keyStates['d'])
    {
        tankturn--; 	//move right
    }

	if(keyStates['q'])
    {
        turretturn++; 	//rotate turret left
    }

	if(keyStates['e'])
    {
        turretturn--; 	//rotate turret right
    }

	if(keyStates['-'])
    {
        zoom++; 	//camera zoom in
    }

	if(keyStates['='])
    {
        zoom--; 	//camera zoom out
    }

}

//! Mouse Interaction
void mouse(int button, int state, int x, int y)
{
	if (camera == 0)
	{
		cameraManip.handleMouse(button, state,x,y);
	}
    glutPostRedisplay(); 
}

//! Motion
void motion(int x, int y)
{
    if (camera == 0)
	{
		cameraManip.handleMouseMotion(x,y);
	}
    glutPostRedisplay(); 
}

//! Timer Function
void Timer(int value)
{
	//Call function again after 10 milli seconds
	glutTimerFunc(10, Timer, 0);
	t = t + 0.015;
	glutPostRedisplay();
}

//! Render 2D text function
void render2dText(std::string text, float r, float g, float b, float x, float y)
{
	glColor3f(r,g,b);
	glRasterPos2f(x, y); // window coordinates
	for(unsigned int i = 0; i < text.size(); i++)
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}

//! Read in stages from input file
int initmap()
{
	using namespace std;

	ifstream fp("input.txt");
	
	if (!fp){
		cout << "Error in reading file" << endl;
		return -1;
	}

	for (int x=0; x<10; x++){
		for (int y=0; y<10; y++){
			fp >> Field[x][y];
		}
	}
}

int chooselevel()
{
	using namespace std;

		//Tank
		int tankmove = 0;
		int tankturn = 0;
		int turretturn = 0;

		float tank_scale = 0.15;
		float tank_x = 0;
		float tank_y = 0;
		float ball_x = 0;
		float ball_y = 0;

		int shoot = 0;
		int projectiletime = 0;
		//Time

		//Zoom
		int zoom = 2;

		//Game
		int gamestate = 0;
		int totalcoin = 0;

		int timeremaining = 1;
		int currentcoin = 0;
		int camera = 1;
		int menu = 0;

		float t = 0;

	if (level == 2)
	{

		
		ifstream fp("input1.txt");
	
		if (!fp){
			cout << "Error in reading file" << endl;
			return -1;
		}

		for (int x=0; x<10; x++){
			for (int y=0; y<10; y++){
				fp >> Field[x][y];
			}
		}
	}
	else if(level == 3)
	{

		ifstream fp("input2.txt");
	
		if (!fp){
			cout << "Error in reading file" << endl;
			return -1;
		}

		for (int x=0; x<10; x++){
			for (int y=0; y<10; y++){
				fp >> Field[x][y];
			}
		}
	}

	else if(level == 4)
	{


		ifstream fp("input3.txt");
	
		if (!fp){
			cout << "Error in reading file" << endl;
			return -1;
		}

		for (int x=0; x<10; x++){
			for (int y=0; y<10; y++){
				fp >> Field[x][y];
			}
		}
	}

	else if(level == 5)
	{


		ifstream fp("input4.txt");
	
		if (!fp){
			cout << "Error in reading file" << endl;
			return -1;
		}

		for (int x=0; x<10; x++){
			for (int y=0; y<10; y++){
				fp >> Field[x][y];
			}
		}
	}

	else if(level == 6)
	{


		ifstream fp("input5.txt");
	
		if (!fp){
			cout << "Error in reading file" << endl;
			return -1;
		}

		for (int x=0; x<10; x++){
			for (int y=0; y<10; y++){
				fp >> Field[x][y];
			}
		}
	}
}

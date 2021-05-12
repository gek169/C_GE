--Begin file.


boing_texture = 0;
extrude_display_list = 0;




function drawMenu()
	glTextSize(3);
	glClear();
	glDrawText("My awesome menu!!!", 0, 0, 0x007777dd);
	build_camview2D({0, 0, 0, 0});
	applyCamera2D();
	--glTranslatef({1.0,0,0});
	glMultMatrixf({
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	});
	if (0 < omg_box(0.1,0.1,0.3,0.3,  0, 0,0, 0x00FFffFF) and omg_cb == 2) then
		print("You Clicked? Aborting...");
		engine_abort();
	end
	if(omg_cb == 2) then
		omg_box(mousex/winSizeX,mousey/winSizeY,   0.03,0.03,  0,0,0, 0x00ff0000);
		print("Click!");
	elseif(omg_cb == 0 or omg_cb == 1) then
		omg_box(mousex/winSizeX,mousey/winSizeY,   0.03,0.03,  0,0,0, 0x0000ff88);
	end
end




function init()
	lMus(1, "WWGW.mp3");
	mPlay(1, -1, 1000);
	boing_texture = loadTexture("boing.png");
	extrude_display_list = buildModelDL("extrude.obj", boing_texture);
	--extrude_display_list = buildModelDL("extrude.obj", 0);
	--Build some entities.
	setGravity({0,-0.001,0});

	
	print("Gravity is...");
	print(getGravity()[1]);
	print(multm4(
	{2,2,2,2,
	2,2,2,2,
	2,2,2,2,
	2,2,2,2},
	{3,3,3,3,
	3,3,3,3,
	3,3,3,3,
	3,3,3,3})[13]);
	print("Init finished!");
	setEnableDepthTest(1);
end

ticker = 0;
function draw()
	setLightingSmoothness(1);
	setEnableLighting(1);
	setEnableLight(0,1);
	setEnableColorMaterial(1);
	setColorMaterialMode(2,0);
	setMaterialProps(	{1,1,1,1},
						{1,1,1,1},
						{1,1,1,1},
						{0,0,0,0},
						30.0);
	setLightProps(0,3, 0,1,-0.6,0); --position
	setLightProps(0,1, 10,10,10,0); --diffuse
	setLightProps(0,0, 0,0,0,0); --ambient
	ticker = ticker + 0.01666666;
	setCullingMode(1,1);
	glClear();
	build_camview({0,0,-10},
					{0,0,1},
					{0,1,0});
	setPerspective(70, winSizeX/winSizeY, 0.1, 100);
	applyCamera3D();
	glTranslatef({0,math.sin(ticker),4});
	glScalef({1,2,1});
	callList(extrude_display_list);
	print("This is finishing, btw");
	--stepChadWorld(2);
	--renderChadWorld();
	setEnableLighting(0);
end

function cleanup()
	dMus(1);
	print("I was asked to clean up my mess.");
end

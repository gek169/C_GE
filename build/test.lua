boing_texture = 0;
boing_display_list = 0;
platform_display_list = 0;
wall_display_list = 0;
ticker = 0.0;


do
--closure.
	math.randomseed(os.time())
	local last_ent = 0;
	function createBall()
		entity_setDL(last_ent, boing_display_list);
		print("Init reached here.");
		entity_setMass(last_ent, 10.0);
		entity_setFriction(last_ent, 0.999);
		entity_setAirFriction(last_ent, 0.999);
		entity_setBounciness(last_ent, 0);
		entity_setShape(last_ent,
			{math.random(-800,800)/1000.0 + 1.0,
			math.random(-200,200)/1000.0 - 1.0, 0, 20/winSizeX,
			0,0,0,0});
		entity_setVelocity(last_ent,
			0, 
			0, 0);
		entity_setLocalT(last_ent,
							{1,0,0,0,
							0,1,0,0,
							0,0,1,0,
							0,0,0,1}
					);
		addEntity(last_ent);
		last_ent = last_ent + 1;
	end
	function createWall(x)
		entity_setDL(last_ent, wall_display_list);
		print("Init reached here.");
		entity_setMass(last_ent, 0.0);
		entity_setFriction(last_ent, 0.999);
		entity_setAirFriction(last_ent, 0.999);
		entity_setBounciness(last_ent, 0);
		entity_setShape(last_ent,
			{x,-1.0,0, 0,
			20.0/winSizeX, 1.0, 1000,0});
		entity_setVelocity(last_ent,
			0, 0, 0);
		entity_setLocalT(last_ent,
							{1,0,0,0,
							0,1,0,0,
							0,0,1,0,
							0,0,0,1}
					);
		addEntity(last_ent);
		last_ent = last_ent + 1;
	end
	function createPlatform(y)
		entity_setDL(last_ent, platform_display_list);
		print("Init reached here.");
		entity_setMass(last_ent, 0.0);
		entity_setFriction(last_ent, 0.999);
		entity_setAirFriction(last_ent, 0.999);
		entity_setBounciness(last_ent, 0);
		entity_setShape(last_ent,
			{1.0,y,0, 0,
			1.0,20.0/winSizeY,1000,0});
		entity_setVelocity(last_ent,
			0, 0, 0);
		entity_setLocalT(last_ent,
						{	1,0,0,0,
							0,1,0,0,
							0,0,1,0,
							0,0,0,1}
					);
		addEntity(last_ent);
		last_ent = last_ent + 1;
	end
end

function drawMenu()
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
		omg_box(mousex/winSizeX,mousey/winSizeY,   0.01,0.01,  0,0,0, 0x00ff0000);
		print("Click!");
	elseif(omg_cb == 0 or omg_cb == 1) then
		omg_box(mousex/winSizeX,mousey/winSizeY,   0.01,0.01,  0,0,0, 0x0000ff88);
	end
end




function init()
	lMus(1, "WWGW.mp3");
	mPlay(1, -1, 1000);
	boing_texture = loadTexture("boing.png");
	boing_display_list = buildSpriteDL(20.0/winSizeX, 20.0/winSizeY, boing_texture);
	platform_display_list = buildRectangleDL(1.0, 10.0/winSizeY, 	0.0, 1.0, 0.0);
	wall_display_list = buildRectangleDL(20.0/winSizeY,1.0, 	0.0, 1.0, 0.0);
	--Build some entities.
	setGravity({0,-0.001,0});
	print("Gravity IS::::");
	print(getGravity()[1]);
	setMS(200);
	createBall();
	createBall();
	createPlatform(-1.2);
	createPlatform(0);
	createWall(2.0);
	createWall(0.0);
	print("Init finished!");
end


function draw()
	ticker = ticker + 0.016666666;
	local sin = math.sin;
		build_camview2D({0, sin(ticker), 0, 0});
		applyCamera2D();
	if(button1 > 0) then
		createBall();
	end
	stepChadWorld(2);
	renderChadWorld();
end

function cleanup()
	dMus(1);
	print("I was asked to clean up my mess.");
end

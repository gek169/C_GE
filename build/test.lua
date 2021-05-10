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
			math.random(-1000,1000)/1000.0,
			math.random(-500,500)/1000.0 - 0.1,0, 20/winSizeX,
			0,0,0,0);
		entity_setVelocity(last_ent,
			0, 
			0, 0);
		ld_mat4(0,
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1);
		entity_setLocalT(last_ent,0);

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
			1.0,y,0, 0,
			1000,0.1,1000,0);
		entity_setVelocity(last_ent,
			0, 0, 0);
		ld_mat4(0,
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1);
		entity_setLocalT(last_ent,0);

		addEntity(last_ent);
		last_ent = last_ent + 1;
	end
end

function drawMenu()
	ld_vec3(0, 0, 0, 0);
	build_camview2D();
	applyCamera2D();
	ld_mat4(0,
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1);
	glMultMatrixf(0);
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
	boing_display_list = buildSpriteDL(20, 20, boing_texture);
	platform_display_list = buildRectangleDL(winSizeX, 20, 	0.0, 1.0, 0.0);
	--Build some entities.
	setGravity(0,-0.001,0);
	setMS(200);
	createBall();
	createBall();
	createPlatform(-1.2);
	createPlatform(0);
	print("Init finished!");
end


function draw()
	ticker = ticker + 0.016666666;
	local sin = math.sin;
	ld_vec3(1, 0.1, -0.1, 0);build_camview2D(1); applyCamera2D();
	if(button1 > 0) then
		glPushMatrix();
			ld_vec3(2,
						1.0,0,0);
			glTranslatef(2);
			callList(boing_display_list);
		glPopMatrix();
		createBall();
	end
	stepChadWorld(2);
	renderChadWorld();
end

function cleanup()
	dMus(1);
	print("I was asked to clean up my mess.");
end

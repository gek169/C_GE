
boing_texture = 0;
boing_display_list = 0;

function drawMenu()
	ld_vec3(0, 0, 0, 0);
	build_camview2D(0);
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
		print("Click!");
	end
end




function init()
	lmus(1, "WWGW.mp3");
	mplay(1, -1, 1000);
	boing_texture = loadTexture("boing.png");
	boing_display_list = buildSpriteDisplayList(20, 20, boing_texture);
end


function draw()
	local sin = math.sin;
	ld_vec3(1, 0.1, -0.1, 0);
	build_camview2D(1);
	applyCamera2D();
	glPushMatrix();
		ld_vec3(12,0,sin(tpassed),0);
		glTranslatef(12);
		callList(boing_display_list);
	glPopMatrix();
	glPushMatrix();
		ld_vec3(1,1.0,0,0);
		glTranslatef(1);
		callList(boing_display_list);
	glPopMatrix();
end

function cleanup()
	print("I was asked to clean up my mess.");
end

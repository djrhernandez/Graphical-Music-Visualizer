struct Fog {
	bool	foggy;
	GLuint	filter;
	GLuint	fogFilter;
	GLfloat	color[4];

	Fog() {
		foggy = false;
		filter = 0;
		fogFilter = 0;
		vecMake(0.2, 0.2, 0.2, color);
		color[3] = 0.75;
	}
};

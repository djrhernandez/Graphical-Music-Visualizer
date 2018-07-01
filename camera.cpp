class Camera {
	public:
		Vec from, at, up, eye;
		Flt angle;
		
		Camera() {
			angle = 45.0;
			vecMake(0.0, 45.0, 50.0, from);
			vecMake(0.0,  5.0,  0.0, at);
			vecMake(0.0,  1.0,  0.0, up);
			vecMake(0.0,  0.0,  0.0, eye);
		}
} cam;

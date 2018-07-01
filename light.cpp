class Light {
	public:
		Vec amb, diff, spec, shine, pos;

		Light () {
			vecMake(  0.4,  0.4,   0.4, amb);
			vecMake(  0.8,  0.8,   0.8, diff);
			vecMake(  1.0,  1.0,   1.0, spec);
			vecMake(  0.8,  0.8,   0.8, shine);
			vecMake(100.0, 25.0, 100.0, pos);
		}
} light;

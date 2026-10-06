class OBLServerMarker : OBLMarker {
	
	bool toSurface, display3d, displayMap, striked, temp;
	float radius;
	
	override bool ReadFromCtx(ParamsReadContext ctx) {
		if (!super.ReadFromCtx(ctx))
			return false;
		if (!ctx.Read(toSurface))
			return false;
		if (!ctx.Read(display3d))
			return false;
		if (!ctx.Read(displayMap))
			return false;
		if (!ctx.Read(striked))
			return false;
		if (!ctx.Read(radius))
			return 0;
		if (!ctx.Read(temp))
			return false;
		return true;
	}
	
	void InitFromOBLMarker(OBLMarker marker) {
		this.type = marker.type;
		this.uid = marker.uid;
		this.name = marker.name;
		this.icon = marker.icon;
		this.position = marker.position;
		this.currentSubgroup = marker.currentSubgroup;
		this.colorA = marker.colorA;
		this.colorR = marker.colorR;
		this.colorG = marker.colorG;
		this.colorB = marker.colorB;
		toSurface = true;
		display3d = true;
		displayMap = true;
		striked = false;
		temp = false;
		radius = 0;
	}
	
	override void WriteToCtx(ParamsWriteContext ctx) {
		super.WriteToCtx(ctx);
		ctx.Write(toSurface);
		ctx.Write(display3d);
		ctx.Write(displayMap);
		ctx.Write(striked);
		ctx.Write(radius);
		ctx.Write(temp);
	}
	
	override void InitMarker() {
		super.InitMarker();
		if (toSurface)
			position[1] = GetGame().SurfaceY(position[0], position[2]);
	}
	
	override bool UpdateMarkerClient() {
		if (display3d)
			return super.UpdateMarkerClient();
		SetVisibleOnScreen(false);
		return false;
	}
	
	static int staticMarkerUID = 100;
	void Init(string namee, vector pos, string ic, int r, int g, int b, bool toSuf = true, bool disp3d = true, bool dispMap = true) {
		type = OBLMarkerType.SERVER_STATIC;
		uid = Math.RandomInt(200, int.MAX - 1);
		name = namee;
		position = pos;
		icon = ic;
		colorA = 255;
		colorR = r;
		colorG = g;
		colorB = b;
		toSurface = toSuf;
		display3d = disp3d;
		displayMap = dispMap;
		temp = true;
	}

	void SetRadius(float radius_, int color_, bool striked_ = true, bool disp3d = true, bool sync = true) {
		display3d = disp3d;
		striked = striked_;
		radius = radius_;
		if (sync)
			OBLStaticMarkerManager.Get().SendMarkerRefreshRPC();
	}

	override bool SetColorARGB(int a, int r, int g, int b) {
		if (colorA != a || colorR != r || colorG != g || colorB != b) {
			colorA = a;
			colorR = r;
			colorG = g;
			colorB = b;
			OBLStaticMarkerManager.Get().SendMarkerRefreshRPC();
			return true;
		}
		return false;
	}

	bool DrawCircle() {
		if (striked)
			return true;
		return false;
	}
}
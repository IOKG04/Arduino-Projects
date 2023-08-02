using SixLabors.ImageSharp;
using SixLabors.ImageSharp.Processing;
using RGB = SixLabors.ImageSharp.PixelFormats.Rgb24;
using System.IO;

namespace ImgProcessor;

class Program{
	static void Main(string[] args){
		int n = int.Parse(args[0]);
		string outps = "{\n";
		for(int i = 0; i < n; i++){
			Console.WriteLine("Calculating " + i);
			Image<RGB> inp = (Image<RGB>)Image.Load("../Frames/BA" + (i + 1).ToString("0000") + ".png");
			inp.Mutate(x => x.Resize(12, 8, KnownResamplers.Box)); //Test Different Resize Algorythms TODO
			UInt32[] outp = new UInt32[3];
			for(int x = 0; x < inp.Width; x++){
				for(int y = 0; y < inp.Height; y++){
					if(inp[x, y].R > 127) outp[(x + y * 12) / 32] |= ((uint)1) << (31 - ((x + y * 12) % 32));
				}
			}
			outps += "  {" + outp[0] + ", " + outp[1] + ", " + outp[2] + "},\n";
		}
		outps += "}";
		File.WriteAllText("../BadAppleBin.txt", outps);
	}
}

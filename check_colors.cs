using System;
using System.Drawing;

class Program {
    static void Main() {
        Bitmap img = new Bitmap("textures/duzydemon.png");
        Console.WriteLine($"Image size: {img.Width}x{img.Height}");
        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 10; x++) {
                Color c = img.GetPixel(x, y);
                if (c.R != 255 || c.G != 0 || c.B != 255) {
                    Console.WriteLine($"Pixel at {x},{y}: R={c.R} G={c.G} B={c.B}");
                }
            }
        }
        
        // Let's also scan the middle of the first column
        for (int y = 50; y < 60; y++) {
             Color c = img.GetPixel(0, y);
             if (c.R != 255 || c.G != 0 || c.B != 255) {
                    Console.WriteLine($"Left border pixel at 0,{y}: R={c.R} G={c.G} B={c.B}");
             }
        }
    }
}

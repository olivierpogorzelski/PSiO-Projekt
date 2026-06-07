using System;
using System.Drawing;

class Program {
    static void Main() {
        try {
            using (Bitmap b = new Bitmap("textures/czacha.png")) {
                Console.WriteLine($"WYMIARY_CZASZKI: {b.Width}x{b.Height}");
            }
        } catch (Exception e) {
            Console.WriteLine("Blad: " + e.Message);
        }
    }
}

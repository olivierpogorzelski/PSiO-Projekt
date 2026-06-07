using System;
using System.IO;
using System.Text.RegularExpressions;

public class Fixer {
    public static void Main() {
        var files = Directory.GetFiles(@"c:\Users\oli-v\Documents\PSiO-Projekt", "*.*", SearchOption.AllDirectories);
        foreach(var f in files) {
            if(!f.EndsWith(".cpp") && !f.EndsWith(".hpp")) continue;
            
            byte[] bytes = File.ReadAllBytes(f);
            string text = "";
            try { text = new System.Text.UTF8Encoding(false, true).GetString(bytes); }
            catch { text = System.Text.Encoding.GetEncoding(1250).GetString(bytes); }
            
            text = Regex.Replace(text, @"//.*", m => {
                return m.Value.ToLower()
                    .Replace("zeby", "żeby").Replace("ž", "ż").Replace("miesci", "mieści")
                    .Replace("sciana", "ścianą").Replace("kamera", "kamerą").Replace("wspolna", "wspólna")
                    .Replace("wspolne", "wspólne").Replace("wrogow", "wrogów").Replace("przedmiotow", "przedmiotów")
                    .Replace("pociskow", "pocisków").Replace("petla", "pętla").Replace("rysujaca", "rysująca")
                    .Replace("metoda", "metodą").Replace("wlasciwa", "właściwa").Replace("gorna", "górna")
                    .Replace("dolna", "dolna").Replace("przezroczystosc", "przezroczystość").Replace("obrazen", "obrażeń")
                    .Replace("polozenia", "położenia").Replace("wspolrzednych", "współrzędnych").Replace("uzywamy", "używamy")
                    .Replace("zycia", "życia").Replace("plyn", "płyn").Replace("wlasne", "własne").Replace("otwor", "otwór")
                    .Replace("proznia", "próżnia").Replace("scian", "ścian").Replace("sciany", "ściany").Replace("podloga", "podłoga")
                    .Replace("czysci", "czyści").Replace("domyslne", "domyślne").Replace("zadzialalo", "zadziałało")
                    .Replace("przypisac", "przypisać").Replace("lukiem", "łukiem").Replace("glownego", "głównego")
                    .Replace("uzywajac", "używając").Replace("zaleznie", "zależnie").Replace("szerokosc", "szerokość")
                    .Replace("dolu", "dołu").Replace("gora", "góra").Replace("wyjdz", "wyjdź").Replace("wspolnej", "wspólnej")
                    .Replace("obražen", "obrażeń");
            });
            File.WriteAllText(f, text, new System.Text.UTF8Encoding(false));
        }
        Console.WriteLine("Gotowe!");
    }
}

$utf8NoBom = New-Object System.Text.UTF8Encoding $false
$enc1250 = [System.Text.Encoding]::GetEncoding(1250)

function Read-Text-Auto($path) {
    $bytes = [System.IO.File]::ReadAllBytes($path)
    try {
        $strictUtf8 = New-Object System.Text.UTF8Encoding $false, $true
        return $strictUtf8.GetString($bytes)
    } catch {
        return $enc1250.GetString($bytes)
    }
}

$files = Get-ChildItem -Path "c:\Users\oli-v\Documents\PSiO-Projekt" -Include *.cpp,*.hpp -Recurse

$replacements = @{
    "\bžeby\b" = "żeby"
    "\bkaždej\b" = "każdej"
    "\bobražeń\b" = "obrażeń"
    "\bužytkownika\b" = "użytkownika"
    "\bužywając\b" = "używając"
    "\bnałožyć\b" = "nałożyć"
    "\bjuž\b" = "już"
    "\bpołoženia\b" = "położenia"
    "\bzaležnie\b" = "zależnie"
    "\bdługosc\b" = "długość"
    "\bpredkosc\b" = "prędkość"
    "\bszerokosc\b" = "szerokość"
    "\bwysokosc\b" = "wysokość"
    "\bodleglosc\b" = "odległość"
    "\bzaleznie\b" = "zależnie"
    "\bwyjdz\b" = "wyjdź"
    "\bmiesci\b" = "mieści"
    "\bkamera\b" = "kamerą"
    "\bsciana\b" = "ścianą"
    "\bwspolna\b" = "wspólna"
    "\bwspolne\b" = "wspólne"
    "\bwspolnej\b" = "wspólnej"
    "\bwrogow\b" = "wrogów"
    "\bprzedmiotow\b" = "przedmiotów"
    "\bpociskow\b" = "pocisków"
    "\bpetla\b" = "pętla"
    "\brysujaca\b" = "rysująca"
    "\bmetoda\b" = "metodą"
    "\bwlasciwa\b" = "właściwa"
    "\bgorna\b" = "górna"
    "\bdolna\b" = "dolna"
    "\bgora\b" = "góra"
    "\bdolu\b" = "dołu"
    "\bprzezroczystosc\b" = "przezroczystość"
    "\bobrazen\b" = "obrażeń"
    "\buderzen\b" = "uderzeń"
    "\bpolozenia\b" = "położenia"
    "\bwspolrzednych\b" = "współrzędnych"
    "\bwspolrzedne\b" = "współrzędne"
    "\buzywamy\b" = "używamy"
    "\bzycia\b" = "życia"
    "\bplyn\b" = "płyn"
    "\bprzezroczysta\b" = "przezroczysta"
    "\bwlasne\b" = "własne"
    "\botwor\b" = "otwór"
    "\bproznia\b" = "próżnia"
    "\bscian\b" = "ścian"
    "\bsciany\b" = "ściany"
    "\bpodloga\b" = "podłoga"
    "\bpodlogi\b" = "podłogi"
    "\bczysci\b" = "czyści"
    "\bdomyslne\b" = "domyślne"
    "\bzadzialalo\b" = "zadziałało"
    "\bprzypisac\b" = "przypisać"
    "\blukiem\b" = "łukiem"
    "\bglownego\b" = "głównego"
    "\bzabezpieczajacy\b" = "zabezpieczający"
    "\bzeby\b" = "żeby"
    "\bjesli\b" = "jeśli"
    "\bpzniej\b" = "później"
    "\bkolo\b" = "koło"
    "\bwrogw\b" = "wrogów"
    "\bprzedmiotw\b" = "przedmiotów"
    "\bwsplnej\b" = "wspólnej"
    "\bpociskw\b" = "pocisków"
    "\bprzywrcic\b" = "przywrócić"
    "\bdl\b" = "dół"
    "\botwr\b" = "otwór"
    "\bgry\b" = "góry"
    "\bpzniej\b" = "później"
    "ž" = "ż"
}

foreach ($f in $files) {
    $text = Read-Text-Auto $f.FullName
    
    $evaluator = [System.Text.RegularExpressions.MatchEvaluator] {
        param($m)
        $comment = $m.Value.ToLower()
        
        foreach ($key in $replacements.Keys) {
            $comment = [System.Text.RegularExpressions.Regex]::Replace($comment, $key, $replacements[$key], [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)
        }
        return $comment
    }
    
    $newText = [System.Text.RegularExpressions.Regex]::Replace($text, "//.*", $evaluator)
    
    [System.IO.File]::WriteAllText($f.FullName, $newText, $utf8NoBom)
}
Write-Host "Zrobione!"

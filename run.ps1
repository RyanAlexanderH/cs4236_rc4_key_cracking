$sw = [Diagnostics.Stopwatch]::StartNew()

for ($i = 0; $i -lt 60; $i++) {
	if ($i -lt 10) {
		write-output "Finding key for A0$i.data";
		invoke-expression "./findKey.exe A0$i.data";
	} else {
		write-output "Finding key for A$i.data";
		invoke-expression "./findKey.exe A$i.data";
	}
	write-output "`n---------------------------------------`n"
}

$sw.Stop()
$sw.Elapsed
Read-Host -Prompt "Press Enter to exit"
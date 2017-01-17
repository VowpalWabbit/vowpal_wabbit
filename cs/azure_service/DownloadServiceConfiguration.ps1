$mcURL = Read-Host -Prompt "Management Center URL" 
$storageAccountKey = Read-Host "Storage Account Key" 

$storageAccountKey = [uri]::EscapeDataString($storageAccountKey)
Invoke-WebRequest -Uri "$($mcURL)Deployment/GenerateTrainerConfig?key=$storageAccountKey" -OutFile ServiceConfiguration.Local.cscfg

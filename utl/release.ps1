$token = "b07595278db1d21048347b802901bb0fafb8ae04"
$repo = "eisber"
$version = "8.0.0.65" 
$headers = @{ Authorization = "token $token" }

$body = @{ tag_name = "v$version"; name = "v$version"; body = "v$version"; draft = $false; prerelease = $false }
$release = Invoke-RestMethod -Headers $headers  https://api.github.com/repos/$repo/vowpal_wabbit/releases -Method Post -Body (ConvertTo-Json $body)

$url = ($release.upload_url.Replace("{?name,label}", "?name=VowpalWabbit-$version.msi"))
$asset = Invoke-RestMethod -Headers $headers -Method Post -ContentType "application/zip" -InFile ..\vowpalwabbit\x64\Release\VowpalWabbit.msi $url


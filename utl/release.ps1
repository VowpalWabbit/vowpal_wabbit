param([string]$githubToken, [string]$repo = "eisber")

$headers = @{ Authorization = "token $githubToken" }

$version = ([xml](Get-Content -Path cs\version.props)).Project.PropertyGroup.VowpalWabbitAssemblyVersion

$body = @{ tag_name = "v$version"; name = "v$version"; body = "v$version"; draft = $false; prerelease = $false }
$release = Invoke-RestMethod -Headers $headers  https://api.github.com/repos/$repo/vowpal_wabbit/releases -Method Post -Body (ConvertTo-Json $body)

$url = ($release.upload_url.Replace("{?name,label}", "?name=VowpalWabbit-$version.msi"))
$asset = Invoke-RestMethod -Headers $headers -Method Post -ContentType "application/zip" -InFile vowpalwabbit\x64\Release\VowpalWabbit.msi $url

$url = ($release.upload_url.Replace("{?name,label}", "?name=VowpalWabbit-AzureCloudService-$version.cspkg"))
$asset = Invoke-RestMethod -Headers $headers -Method Post -ContentType "application/zip" -InFile cs\azure_service\bin\Release\app.publish\azure_service.cspkg $url
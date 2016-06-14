param([string]$githubToken, [string]$repo = "eisber")

Get-ChildItem *.cspkg -Recurse

$version = ([xml](Get-Content -Path cs\version.props)).Project.PropertyGroup.VowpalWabbitAssemblyVersion

$body = @{ tag_name = "v$version"; name = "v$version"; body = "v$version"; draft = $false; prerelease = $false }
$release = Invoke-RestMethod -Headers @{ Authorization = "token $githubToken" }  https://api.github.com/repos/$repo/vowpal_wabbit/releases -Method Post -Body (ConvertTo-Json $body)

try {
$url = ($release.upload_url.Replace("{?name,label}", "?name=VowpalWabbit-AzureCloudService-$version.cspkg"))
$asset = Invoke-RestMethod -Headers @{ Authorization = "token $githubToken" } -Method Post -ContentType "application/zip" -InFile cs\azure_service\bin\Release\app.publish\azure_service.cspkg $url
}
catch [Exception] {
   [Exception]$e = $_;
   for ($i=1;($i -le 10) -and ($e -ne $null);$i++) {
        Write-Host $e.Message $e.StackTrace
        $e = $e.InnerException
   }
}

$url = ($release.upload_url.Replace("{?name,label}", "?name=VowpalWabbit-$version.msi"))
$asset = Invoke-RestMethod -Headers @{ Authorization = "token $githubToken" } -Method Post -ContentType "application/zip" -InFile vowpalwabbit\x64\Release\VowpalWabbit.msi $url

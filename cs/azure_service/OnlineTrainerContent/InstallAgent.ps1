#########################################################
#														#
#	Copyright (C) Microsoft. All rights reserved.		#
#														#
#########################################################

# Globals

$RootDir = split-path $MyInvocation.MyCommand.Path
$RootDrive = [System.IO.Path]::GetPathRoot($RootDir)
$CacheDir = Join-Path $RootDrive "AppInsightsAgent"
$InfoLogPath = Join-Path $CacheDir "install.agent.info.log"
$ErrorLogPath = Join-Path $CacheDir "install.agent.error.log"
New-Item $CacheDir -type directory -ErrorAction SilentlyContinue

[System.Diagnostics.Stopwatch] $InstallationStopWatch = New-Object System.Diagnostics.StopWatch

$global:lock = $null

# Constants

$agentMsi = "ApplicationInsightsAgent.msi"

# Infrastructure functions

function Log-Message
{
	param (
		[string] $message
	)

	Add-Content $InfoLogPath ("{0}: {1}" -f (Get-Date), $message)
}

function Log-Error
{
	param (
		[string] $message
	)

	Add-Content $InfoLogPath ("{0}: {1}" -f (Get-Date), $message)
	Add-Content $ErrorLogPath ("{0}: {1}" -f (Get-Date), $message)
}

function Get-ConfigSettingValue
{
	param (
		[string] $settingName,
		[string] $defaultSettingValue
	)

	$settingValue = [Environment]::GetEnvironmentVariable($settingName)
	if(-NOT $settingValue)
	{
		Log-Message "$settingName setting is not defined in Environment, default to $defaultSettingValue"
		$settingValue = $defaultSettingValue
	}

	Log-Message "GetEnvironmentVariable $settingName value: $settingValue"
	return $settingValue.Trim('/')
}

function TryV1
{
	param (
		[ScriptBlock] $Command = $(throw "The parameter -Command is required."),
		[ScriptBlock] $Catch   = { throw $_ },
		[ScriptBlock] $Finally = { }
	)

	& {
		$local:ErrorActionPreference = "SilentlyContinue"

		trap
		{
			trap
			{
				& {
					trap { throw $_ }
					& $Finally
				}

				throw $_
			}

			$_ | & { & $Catch }
		}

		& $Command
	}
 
	& {
		trap { throw $_ }
		& $Finally
	}
}

function Retry
{
	param (
		[ScriptBlock] $RetryCommand,
		[Int] $MaxAttempts = 10,
		[Int] $SleepInSecondsBeforeRetry = 3
	)

	for ($attempts = 0; $attempts -le $MaxAttempts; $attempts++)
	{
		TryV1 {
			& $RetryCommand
			break
		} -Catch {
			if($attempts -lt $MaxAttempts)
			{
				Log-Message "Attempt:$attempts Exception Occured. Sleeping $SleepInSecondsBeforeRetry second and Retrying..."
				Log-Message $_
				Log-Message $_.InvocationInfo.PositionMessage
				Start-Sleep -Seconds $SleepInSecondsBeforeRetry
			}
			else
			{
				throw $_
			}
		}
	}
}

function Execute-CriticalSection
{
	param (
		[ScriptBlock] $CriticalSection
	)
 
	Get-Lock
	& $CriticalSection
	Release-Lock
}

function Get-Lock
{
	Log-Message "Trying to acquire lock"
	$numberOfAttempts = 600
	for ($attempts = 0; $attempts -lt $numberOfAttempts; $attempts++)
	{
		TryV1 {
			$global:lock = New-object System.IO.FileStream("$CacheRoot\lock.startup.task.sem", [System.IO.FileMode]::OpenOrCreate, [System.IO.FileAccess]::Read, [System.IO.FileShare]::None)
			Log-Message "Acquired lock for continuing startup task"
			break
		} -Catch {
			if($attempts -eq $numberOfAttempts)
			{
				Log-Message "Could not acquire lock for continuing startup task even after $numberOfAttempts attempts!"
				throw $_
			}

			Log-Message "Could not acquire lock for continuing startup task. Sleeping and Retrying..."
			Start-Sleep -Seconds 1
		}
	}
}

function Release-Lock
{
	Log-Message "Releasing lock"
	$global:lock.Dispose()
}

# Settings functions

function Get-AgentDownloadLink
{
	return Get-ConfigSettingValue "ApplicationInsightsAgent.DownloadLink" "http://go.microsoft.com/fwlink/?LinkID=522371&clcid=0x409"
}

# Top level functions

$definition = @'
[DllImport("msi.dll", CharSet = CharSet.Auto)]
public static extern int MsiQueryProductState(string product);
'@
$msi = Add-Type -MemberDefinition $definition -Name 'Msi' -Namespace 'Msi' -PassThru

function Check-MsiInstallation
{
	param (
		[string] $msiPath
	)
	
	Log-Message "Start - Check if MSI is installed."

	$installer = New-Object -ComObject WindowsInstaller.Installer
	$msiPathObject = @($msiPath, 0)
	$database = $installer.GetType().InvokeMember("OpenDatabase", "InvokeMethod", $null, $installer, $msiPathObject)
	$view = $database.GetType().InvokeMember("OpenView","InvokeMethod", $null, $database, "SELECT Value FROM Property WHERE Property = 'ProductCode'")
	$null = $view.GetType().InvokeMember("Execute", "InvokeMethod", $null, $view, $null)
	$record = $view.GetType().InvokeMember("Fetch", "InvokeMethod", $null, $view, $null)
	$productCode = $record.GetType().InvokeMember("StringData", "GetProperty", $null, $record, 1)
	$null = $view.GetType().InvokeMember("Close", "InvokeMethod", $Null, $view, $Null)
	Log-Message "Obtained product code $productCode"
	$state = $msi::MsiQueryProductState($productCode)
	Log-Message "Obtained product state $state"

	# To determine if the product is already installed, check for one of the following results:
	# INSTALLSTATE_LOCAL        =  3,  // installed on local drive
	# INSTALLSTATE_SOURCE       =  4,  // run from source, CD or net
	# INSTALLSTATE_DEFAULT      =  5,  // use default, local or source
	$isInstalled = $state -ge 3
	if ($isInstalled) {
		Log-Message "Agent MSI is already installed."
	} else {
		Log-Message "Agent MSI is not installed."
	}

	Log-Message "End - Check if Agent MSI is installed."

	return $isInstalled
}

function Download-AgentMsi
{
	param (
		[string] $msiPath
	)

	Log-Message "Start - Agent MSI download process."

	$downloadLink = Get-AgentDownloadLink
	Retry {
		Log-Message "Downloading from $downloadLink"
		$wc = New-Object System.Net.WebClient
		$wc.DownloadFile($downloadLink, $msiPath)
	}

	Log-Message "End - Agent MSI download process."
}

function Invoke-AgentMsiInstallation
{
	param (
		[string] $msiPath		
	)
	$msiLogPath = Join-Path $CacheDir "install.agent.log"
	Log-Message "Start - Agent MSI installation process."

	msiexec /i $msiPath  /qn /lv+ $msiLogPath | Out-Null

	Log-Message "End - Agent MSI installation process."
}

function Restart-IIS
{
	Log-Message "Start - Restart IIS."

	$sitePath = Join-Path $RootDrive "\sitesroot"
	Log-Message "Check if path exists: $sitePath"
	if (Test-Path $sitePath)
	{
		Log-Message "This is a web role, restarting IIS."
		Restart-Service W3SVC,WAS -force
	}
	else
	{
		Log-Message "This is a worker role, will not restart IIS."
	}

	Log-Message "End - Restart IIS."
}

# Main()

TryV1 {
	$InstallationStopWatch.Start()
	
	$msiPath = Join-Path $CacheDir $agentMsi
	Log-Message "Obtained MSI path $msiPath"

	# always download the MSI if not exist in path
	$testMsiPath = Test-Path $msiPath
	if ($testMsiPath -eq $false) {
		Download-AgentMsi $msiPath
	} else {
		Log-Message "Skip Agent MSI download."
	}

	# only install MSI if not already installed
	$isMsiInstalled = Check-MsiInstallation $msiPath
	if ($isMsiInstalled -eq $false) {
		Execute-CriticalSection {
			Invoke-AgentMsiInstallation $msiPath 
			Restart-IIS
		}
	} else {
		Log-Message "Skip Agent MSI installation."
	}
} -Catch {
	Log-Error "Unhandled Exception Occured. Terminating the startup task"
	Log-Error $_
	Log-Error $_.InvocationInfo.PositionMessage
	throw $_
} -Finally {
	$InstallationStopWatch.Stop()
}
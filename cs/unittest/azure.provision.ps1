Login-AzureRmAccount

Select-AzureRmSubscription -SubscriptionId "FILL ME IN"

$rg = 'FILL ME IN'
New-AzureRmResourceGroup -Name $rg -Location "East US"

# -DeploymentDebugLogLevel All
New-AzureRmResourceGroupDeployment -ResourceGroupName $rg -TemplateFile .\azuredeploy.json 

# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant::Config.run do |config|
  # Base box for us is precise32
  config.vm.box = "precise32"

  # The url from where the 'config.vm.box' box will be fetched if it
  # doesn't already exist on the user's system.
  config.vm.box_url = "http://files.vagrantup.com/precise32.box"

  # Provision using Puppet
  config.vm.provision :puppet do |puppet|
    puppet.manifests_path = "scripts"
    puppet.manifest_file  = "tp_environment.pp"
  end
end

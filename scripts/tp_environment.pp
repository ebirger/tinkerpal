group { "puppet":
  ensure => "present",
}

class tp_js_pre_req {
  exec { 'apt-get update':
    command => '/usr/bin/apt-get update'
  }

  package { "gcc":
    ensure => present,
  }

  package { "make":
    ensure => present,
  }
  
  package { "gperf":
    ensure => present,
  }
  
  package { "bison":
    ensure => present,
  }

  package { "libncurses5-dev":
    ensure => present,
  }
}

class lm4tools_pre_req {
  package { "git-core":
    ensure => present,
  }
  package { "libusb-1.0.0-dev":
    ensure => present,
  }
  package { "pkg-config":
    ensure => present,
  }
}

class summon_arm_toolchain_pre_req {
  package { "flex":
      ensure => present,
  }
  package { "libgmp3-dev":
      ensure => present,
  }
  package { "libmpfr-dev":
      ensure => present,
  }
  package { "libmpc-dev":
      ensure => present,
  }
  package { "autoconf":
      ensure => present,
  }
  package { "texinfo":
      ensure => present,
  }
  package { "build-essential":
      ensure => present,
  }
  package { "libftdi-dev":
      ensure => present,
  }
  package { "zlib1g-dev":
      ensure => present,
  }
  package { "python-yaml":
      ensure => present,
  }
}

class tp_runtime {
  package { "screen":
    ensure => present,
  }
}

include tp_js_pre_req
include lm4tools_pre_req
include summon_arm_toolchain_pre_req
include tp_runtime

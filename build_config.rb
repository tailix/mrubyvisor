MRuby::CrossBuild.new ENV.fetch 'BUILD_NAME' do |conf|
  conf.toolchain :gcc

  conf.gem core: 'mruby-compiler'

  conf.cc.flags      <<
    '-Wall'          <<
    '-Wextra'        <<
    '-ffreestanding' <<
    '-nostdlib'      <<
    '-fno-pic'       <<
    '-fno-stack-protector'

  ENV.fetch('FLAGS').split.map do |flag|
    m = /\A-D(MRB_.*)\z/.match flag
    conf.cc.defines << m[1] if m
  end

  conf.cc.include_paths     << ENV.fetch('INCLUDE_DIR')
  conf.linker.library_paths << ENV.fetch('LIB_DIR')

  conf.archiver.command = ENV.fetch 'CROSS_AR'
  conf.cc.command       = ENV.fetch 'CROSS_CC'
  conf.linker.command   = ENV.fetch 'CROSS_LD'
end

ROOT_DIR = File.expand_path(__dir__).freeze
DEST_DIR = File.join(ROOT_DIR, 'dest').freeze

MRuby::CrossBuild.new 'mrubyvisor' do |conf|
  conf.toolchain :gcc

  conf.disable_presym

  conf.cc.defines   <<
    'MRB_NO_BOXING' <<
    'MRB_NO_FLOAT'  <<
    'MRB_NO_PRESYM' <<
    'MRB_NO_STDIO'

  conf.cc.flags      <<
    '-Wall'          <<
    '-Wextra'        <<
    '-ffreestanding' <<
    '-nostdlib'      <<
    '-fno-pic'       <<
    '-fno-stack-protector'

  conf.cc.include_paths <<     File.join(DEST_DIR, 'include')
  conf.linker.library_paths << File.join(DEST_DIR, 'lib')

  conf.archiver.command = ENV['CROSS_AR']
  conf.cc.command       = ENV['CROSS_CC']
  conf.linker.command   = ENV['CROSS_LD']
end

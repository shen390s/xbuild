{
  description = "xbuild";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = { self, nixpkgs }:

    let
      supportedSystems = [ "x86_64-linux" "i686-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;

      version = nixpkgs.lib.removeSuffix "\n" (builtins.readFile ./version);
      pkgs = nixpkgs.legacyPackages.x86_64-linux;


      xbuildFor = pkgs: let
        # this is only
      in pkgs.callPackage ./xbuild.nix {
        inherit version;
        src = self;
      };

    in

    {
      overlays.default = final: prev: {
        xbuild-new-musl = xbuildFor final.pkgsMusl;
        xbuild-new = xbuildFor final;
      };

      hydraJobs = {
        tarball =
          pkgs.releaseTools.sourceTarball rec {
            name = "xbuild-tarball";
            inherit version;
            versionSuffix = ""; # obsolete
            src = self;
            preAutoconf = "echo ${version} > version";

            # portable configure shouldn't have a shebang pointing to the nix store
            postConfigure = ''
              sed -i '1s|^.*$|#!/bin/sh|' ./configure
            '';
            postDist = ''
              cp README.md $out/
              echo "doc readme $out/README.md" >> $out/nix-support/hydra-build-products
            '';
          };

        coverage =
          (pkgs.releaseTools.coverageAnalysis {
            name = "xbuild-coverage";
            src = self.hydraJobs.tarball;
            lcovFilter = ["*/tests/*"];
          }).overrideAttrs (old: {
            preCheck = ''
              # coverage cflag breaks this target
              NIX_CFLAGS_COMPILE=''${NIX_CFLAGS_COMPILE//--coverage} make -C tests phdr-corruption.so
            '';
          });

        build = forAllSystems (system: self.packages.${system}.xbuild);
        build-sanitized = forAllSystems (system: self.packages.${system}.xbuild.overrideAttrs (old: {
          configureFlags = [ "--with-asan " "--with-ubsan" ];
          # -Wno-unused-command-line-argument is for clang, which does not like
          # our cc wrapper arguments
          CFLAGS = "-Werror -Wno-unused-command-line-argument";
        }));

        # x86_64-linux seems to be only working clangStdenv at the moment
        build-sanitized-clang = nixpkgs.lib.genAttrs [ "x86_64-linux" ] (system: self.hydraJobs.build-sanitized.${system}.override {
          stdenv = nixpkgs.legacyPackages.${system}.llvmPackages_latest.libcxxStdenv;
        });

        # To get mingw compiler from hydra cache
        inherit (self.packages.x86_64-linux) xbuild-win32 xbuild-win64;

        release = pkgs.releaseTools.aggregate
          { name = "xbuild-${self.hydraJobs.tarball.version}";
            constituents =
              [ self.hydraJobs.tarball
                self.hydraJobs.build.x86_64-linux
                self.hydraJobs.build.i686-linux
                # FIXME: add aarch64 emulation to our github action...
                #self.hydraJobs.build.aarch64-linux
                self.hydraJobs.build-sanitized.x86_64-linux
                #self.hydraJobs.build-sanitized.aarch64-linux
                self.hydraJobs.build-sanitized.i686-linux
                self.hydraJobs.build-sanitized-clang.x86_64-linux
              ];
            meta.description = "Release-critical builds";
          };

      };

      checks = forAllSystems (system: {
        build = self.hydraJobs.build.${system};
      });

      devShells = forAllSystems (system: {
        glibc = self.packages.${system}.xbuild;
        default = self.devShells.${system}.glibc;
      } // nixpkgs.lib.optionalAttrs (system != "i686-linux") {
        musl = self.packages.${system}.xbuild-musl;
      });

      packages = forAllSystems (system: let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        xbuild = xbuildFor pkgs;
        default = self.packages.${system}.xbuild;

        # This is a good test to see if packages can be cross-compiled. It also
        # tests if our testsuite uses target-prefixed executable names.
        xbuild-musl-cross = xbuildFor pkgs.pkgsCross.musl64;
        xbuild-netbsd-cross = xbuildFor pkgs.pkgsCross.x86_64-netbsd;

        xbuild-win32 = (xbuildFor pkgs.pkgsCross.mingw32).overrideAttrs (old: {
          NIX_CFLAGS_COMPILE = "-static";
        });
        xbuild-win64 = (xbuildFor pkgs.pkgsCross.mingwW64).overrideAttrs (old: {
          NIX_CFLAGS_COMPILE = "-static";
        });
      } // nixpkgs.lib.optionalAttrs (system != "i686-linux") {
        xbuild-musl = xbuildFor nixpkgs.legacyPackages.${system}.pkgsMusl;
      });

    };
}

{
  description = "Kastle 2 development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs =
    { self, nixpkgs, ... }:
    let
      # Helper to generate a dev shell output for all supported architecture:systems pairings
      forAllSystems =
        function:
        nixpkgs.lib.genAttrs [
          "aarch64-darwin"
          "aarch64-linux"
          "x86_64-darwin"
          "x86_64-linux"
        ] (system: function nixpkgs.legacyPackages.${system});
    in
    {
      devShells = forAllSystems (
        pkgs:
        {
          default = pkgs.mkShell {
            name = "alchemy-lab-dev-shell";

            buildInputs = [
              pkgs.cmake
              pkgs.doxygen
              pkgs.gcc-arm-embedded
              pkgs.ninja
              pkgs.dfu-util
            ];


            shellHook =
              let
                gum = "${pkgs.gum}/bin/gum";
              in
              ''
                TITLE=$(${pkgs.gum}/bin/gum style \
                  --bold \
                  --foreground '#00fc94' \
                  "Alchemy Lab Development Shell")

                SUBTITLE=$(${pkgs.gum}/bin/gum style \
                  --align center \
                  --width 40 \
                  "Ready for development")

                ${pkgs.gum}/bin/gum style \
                  --align center \
                  --border thick \
                  --padding "1 1" \
                  --border-foreground '#cdb8a5' \
                  "  $TITLE

                  $SUBTITLE"
              '';
          };
        }
      );
    };
}

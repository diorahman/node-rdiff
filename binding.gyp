{
  "targets": [
    {
      "target_name": "rdiff",
      "sources": [
        "rdiff.cc"
      ],
      "link_settings": {
          "libraries": [
              "-lrsync",
              "-lz"
          ]
      },
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
    }
  ]
}
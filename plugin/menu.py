# NtoA
nuke.load('ArnoldRender')
nuke.load('ArnoldStandard')

toolbar = nuke.toolbar("Nodes")
toolbar.addCommand( "NtoA/ArnoldRender", "nuke.createNode('ArnoldRender')")
toolbar.addCommand( "NtoA/ArnoldStandard", "nuke.createNode('ntoaStandard')")

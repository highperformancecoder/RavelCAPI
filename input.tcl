DataCube dc
dc.separator ","
dc.initSpec "PatentsByCountry1980-2011.csv"

dc.loadFile "PatentsByCountry1980-2011.csv"
dc.ravel.handles.@elem 2
dc.ravel.handles(2).description "year"
dc.populateArray dataCube

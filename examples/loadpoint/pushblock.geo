<?xml version="1.0" standalone="no"?>

<!DOCTYPE DDA SYSTEM "geometry.dtd">
<Berkeley:DDA xmlns:Berkeley="http://www.tsoft.com/~bdoolin/dda">

<!-- Geometry section of document -->
<Berkeley:Geometry>

   <Berkeley:Edgenodedist distance="0.01"/>

   <Jointlist>
      <Joint type="1"> 10 9.98 10 20.01 </Joint>
      <Joint type="1"> 9.98 20 50.01 20 </Joint>
      <Joint type="1"> 50 20.01 50 9.98</Joint>
      <Joint type="1"> 50.01 10 9.98 10</Joint>
      <Joint type="1"> 15 19.99 15 25.01 </Joint>
      <Joint type="1"> 14.99 25 20.01 25 </Joint>
      <Joint type="1"> 20 25.01 20 19.99 </Joint>
   </Jointlist>

   <Fixedpointlist>
      <Point> 12      12 </Point>
      <Point> 12      18 </Point>
      <Point> 48      18 </Point>
      <Point> 48      12 </Point>
   </Fixedpointlist>

   <Loadpointlist>
	<Point> 17.5 22.5 </Point>
   </Loadpointlist>

<!--
   <Measuredpointlist>
    	<Point>17.5 22.5 </Point>
   </Measuredpointlist>
-->

</Berkeley:Geometry>
</Berkeley:DDA>

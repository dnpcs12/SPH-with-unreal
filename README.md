# Real-time fluid simmulation in Unreal using smooth-particle hydrodynamics techniques (2021 SW Capstone Design)
 

## SPH <sup>[1](#footnote_1)</sup><sup>[3](#footnote_3)</sup> 
### Basic SPH Formulation
![image](https://user-images.githubusercontent.com/50435598/147324832-45ac5f63-0c88-4054-9e64-526e56135ca9.png)


### Fluid Dynamic
![image](https://user-images.githubusercontent.com/50435598/147325852-b65b7147-0bba-4908-8e9e-07b83e79803b.png)

### Mass-density and Pressure 
![image](https://user-images.githubusercontent.com/50435598/147324872-d26a6a3f-50fb-4e5c-9040-d0cda2c6d4ce.png)

### Internal Force
![image](https://user-images.githubusercontent.com/50435598/147324903-0dabfdd5-163d-413c-bc58-968c45e0098c.png)

### Exteranl Force
![image](https://user-images.githubusercontent.com/50435598/147324983-bf69dae9-de99-4482-ab1b-804ea878f4c1.png)


### Smoothing Kernel
![image](https://user-images.githubusercontent.com/50435598/147325055-c894f39c-432c-405d-b41e-e039ec547360.png)

![image](https://user-images.githubusercontent.com/50435598/147325091-d62d48e1-c569-492b-84db-c23ca320b812.png)

![image](https://user-images.githubusercontent.com/50435598/147325111-08f045b6-e280-4e51-b77b-42dc026a4d52.png)


## Boundary Handling<sup>[4](#footnote_4)</sup>
![image](https://user-images.githubusercontent.com/50435598/147325360-ec1e828d-630f-4c0e-86bb-36fbca9ad4ee.png)


## WCSPH<sup>[5](#footnote_5)</sup>
![image](https://user-images.githubusercontent.com/50435598/147325413-5cbaacb7-11f0-4527-b72a-6a59b9418a8f.png)

## MarchingCube
Marching cubes is a computer graphics algorithm, published in the 1987 SIGGRAPH proceedings by Lorensen and Cline,[1] for extracting a polygonal mesh of an isosurface from a three-dimensional discrete scalar field (the elements of which are sometimes called voxels). <sup>[7](#footnote_6)</sup>




## Result gif
![ezgif com-gif-maker (14)](https://user-images.githubusercontent.com/50435598/147320960-b7167879-8d66-43bc-a3b7-ab531ecf4718.gif)

![ezgif com-gif-maker (10)](https://user-images.githubusercontent.com/50435598/147320971-bb61b287-c46e-451f-a58e-09b0e9121ae9.gif)


References
<ol>
<li id="footnote_1"> M. Müller, D. Charypar, and M. Gross. Particle-based fluid simulation for interactive applications. In Proceedings of the 2003 ACM SIGGRAPH/Eurographics Symposium on Computer Animation, pages 154–159, 2003.</li>
<li id="footnote_2">  Koschier, Dan, Jan Bender, Barbara Solenthaler, and Matthias Teschner. "Smoothed particle hydrodynamics techniques for the physics based simulation of fluids and solids." arXiv preprint arXiv:2009.06944 (2020).</li>
<li id="footnote_3"> David Staubach , "Smoothed Particle Hydrodynamics Real-Time Fluid Simulation Approach." Friedrich-Alexander-Universitaet Erlangen-Nuernberg (2010).</li>
<li id="footnote_4"> Nadir Akinci, Markus Ihmsen, Gizem Akinci, Barbara Solenthaler and Matthias Teshner "Versatile rigid-fluid coupling for incompressible SPH." ACM Transactions on GraphicsVolume 31Issue 4July 2012 Article No.: 62pp 1– (2012).</li>
<li id="footnote_5"> Markus Becker and Matthias Teschner. "Weakly Compressible SPH for Free Surface Flows." Proceedings of the 2007 ACM SIGGRAPH/Eurographics Symposium on Computer Animation, SCA 2007, San Diego, California, USA, August 2-4, 2007. </li>
 <li id="footnote_6"> https://en.wikipedia.org/wiki/Marching_cubes </li>
</ol>

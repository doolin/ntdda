

% Return a plane stress stiffness matrix.
%
% [k] = planestrain(E,nu)
%
% FIXME: Not yet implemented.


function [k] = planestrain(E,nu)

  a1 = E/((1+nu)*(1-(2*nu)));

  k = a1*[
    1-nu  nu   0
     nu  1-nu  0
     0    0   (1-(2*nu))/2
  ];

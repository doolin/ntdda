

% Return a plane stress stiffness matrix.
%
% [k] = planestress(E,nu)

function [k] = planestress(E,nu)

  a1 = E/(1-(nu*nu));

  k = a1*[
    1 nu 0 
    nu 1 0 
    0 0 (1-nu)/2
  ];

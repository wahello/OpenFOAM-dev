/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2024 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "angularOscillatingDisplacementPointPatchVectorField.H"
#include "pointPatchFields.H"
#include "addToRunTimeSelectionTable.H"
#include "Time.H"
#include "polyMesh.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

angularOscillatingDisplacementPointPatchVectorField::
angularOscillatingDisplacementPointPatchVectorField
(
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF,
    const dictionary& dict
)
:
    fixedValuePointPatchField<vector>(p, iF, dict),
    axis_(dict.lookup<vector>("axis", dimless)),
    origin_(dict.lookup<vector>("origin", dimLength)),
    angle0_(dict.lookup<scalar>("angle0", unitRadians)),
    amplitude_(dict.lookup<scalar>("amplitude", unitRadians)),
    omega_(dict.lookup<scalar>("omega", unitRadians/dimTime))
{
    if (!dict.found("value"))
    {
        updateCoeffs();
    }

    if (dict.found("p0"))
    {
        p0_ = vectorField("p0", dimLength, dict, p.size());
    }
    else
    {
        p0_ = p.localPoints();
    }
}


angularOscillatingDisplacementPointPatchVectorField::
angularOscillatingDisplacementPointPatchVectorField
(
    const angularOscillatingDisplacementPointPatchVectorField& ptf,
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF,
    const fieldMapper& mapper
)
:
    fixedValuePointPatchField<vector>(ptf, p, iF, mapper),
    axis_(ptf.axis_),
    origin_(ptf.origin_),
    angle0_(ptf.angle0_),
    amplitude_(ptf.amplitude_),
    omega_(ptf.omega_),
    p0_(mapper(ptf.p0_))
{}


angularOscillatingDisplacementPointPatchVectorField::
angularOscillatingDisplacementPointPatchVectorField
(
    const angularOscillatingDisplacementPointPatchVectorField& ptf,
    const DimensionedField<vector, pointMesh>& iF
)
:
    fixedValuePointPatchField<vector>(ptf, iF),
    axis_(ptf.axis_),
    origin_(ptf.origin_),
    angle0_(ptf.angle0_),
    amplitude_(ptf.amplitude_),
    omega_(ptf.omega_),
    p0_(ptf.p0_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void angularOscillatingDisplacementPointPatchVectorField::map
(
    const pointPatchField<vector>& ptf,
    const fieldMapper& mapper
)
{
    const angularOscillatingDisplacementPointPatchVectorField& aODptf =
        refCast<const angularOscillatingDisplacementPointPatchVectorField>(ptf);

    fixedValuePointPatchField<vector>::map(aODptf, mapper);

    mapper(p0_, aODptf.p0_);
}


void angularOscillatingDisplacementPointPatchVectorField::reset
(
    const pointPatchField<vector>& ptf
)
{
    const angularOscillatingDisplacementPointPatchVectorField& aODptf =
        refCast<const angularOscillatingDisplacementPointPatchVectorField>(ptf);

    fixedValuePointPatchField<vector>::reset(aODptf);

    p0_.reset(aODptf.p0_);
}


void angularOscillatingDisplacementPointPatchVectorField::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    const polyMesh& mesh = this->internalField().mesh()();
    const Time& t = mesh.time();

    scalar angle = angle0_ + amplitude_*sin(omega_*t.value());
    vector axisHat = axis_/mag(axis_);
    vectorField p0Rel(p0_ - origin_);

    vectorField::operator=
    (
        p0Rel*(cos(angle) - 1)
      + (axisHat ^ p0Rel*sin(angle))
      + (axisHat & p0Rel)*(1 - cos(angle))*axisHat
    );

    fixedValuePointPatchField<vector>::updateCoeffs();
}


void angularOscillatingDisplacementPointPatchVectorField::write
(
    Ostream& os
) const
{
    pointPatchField<vector>::write(os);
    writeEntry(os, "axis", axis_);
    writeEntry(os, "origin", origin_);
    writeEntry(os, "angle0", angle0_);
    writeEntry(os, "amplitude", amplitude_);
    writeEntry(os, "omega", omega_);
    writeEntry(os, "p0", p0_);
    writeEntry(os, "value", *this);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePointPatchTypeField
(
    pointPatchVectorField,
    angularOscillatingDisplacementPointPatchVectorField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //

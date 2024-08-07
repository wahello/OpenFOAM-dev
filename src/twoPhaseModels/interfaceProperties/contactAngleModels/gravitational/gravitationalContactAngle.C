/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2023-2024 OpenFOAM Foundation
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

#include "gravitationalContactAngle.H"
#include "uniformDimensionedFields.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace contactAngleModels
{
    defineTypeNameAndDebug(gravitational, 0);
    addToRunTimeSelectionTable(contactAngleModel, gravitational, dictionary);
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::contactAngleModels::gravitational::gravitational(const dictionary& dict)
:
    theta0_(dict.lookup<scalar>("theta0", unitDegrees)),
    gTheta_(dict.lookup<scalar>("gTheta", dimAcceleration)),
    thetaAdv_(dict.lookup<scalar>("thetaAdv", unitDegrees)),
    thetaRec_(dict.lookup<scalar>("thetaRec", unitDegrees))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::contactAngleModels::gravitational::~gravitational()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::tmp<Foam::scalarField>
Foam::contactAngleModels::gravitational::cosTheta
(
    const fvPatchVectorField& Up,
    const vectorField& nHat
) const
{
    const uniformDimensionedVectorField& g =
        Up.db().lookupObject<uniformDimensionedVectorField>("g");

    const scalarField uCoeff(tanh((nHat & g.value())/gTheta_));

    return cos
    (
        theta0_
      + (thetaRec_ - theta0_)*max(uCoeff, scalar(0))
      - (thetaAdv_ - theta0_)*min(uCoeff, scalar(0))
    );
}


void Foam::contactAngleModels::gravitational::write(Ostream& os) const
{
    writeEntry(os, "theta0", unitDegrees, theta0_);
    writeEntry(os, "gTheta", dimAcceleration, gTheta_);
    writeEntry(os, "thetaAdv", unitDegrees, thetaAdv_);
    writeEntry(os, "thetaRec", unitDegrees, thetaRec_);
}


// ************************************************************************* //

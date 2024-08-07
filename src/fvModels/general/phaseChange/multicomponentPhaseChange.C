/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2021-2024 OpenFOAM Foundation
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

#include "multicomponentPhaseChange.H"
#include "basicThermo.H"
#include "multicomponentThermo.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

namespace Foam
{
namespace fv
{
    defineTypeNameAndDebug(multicomponentPhaseChange, 0);
}
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::fv::multicomponentPhaseChange::readCoeffs()
{
    if (species_ != coeffs().lookup<wordList>("species"))
    {
        FatalIOErrorInFunction(coeffs())
            << "Cannot change the species of a " << typeName << " model "
            << "at run time" << exit(FatalIOError);
    }

    energySemiImplicit_ =
        coeffs().lookup<bool>("energySemiImplicit");
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::fv::multicomponentPhaseChange::multicomponentPhaseChange
(
    const word& name,
    const word& modelType,
    const fvMesh& mesh,
    const dictionary& dict,
    const Pair<bool>& fluidThermosRequired
)
:
    phaseChange
    (
        name,
        modelType,
        mesh,
        dict,
        fluidThermosRequired,
        {true, true}
    ),
    species_(coeffs().lookup<wordList>("species")),
    energySemiImplicit_(false)
{
    readCoeffs();
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Foam::tmp<Foam::volScalarField::Internal>
Foam::fv::multicomponentPhaseChange::mDot() const
{
    tmp<volScalarField::Internal> tmDot =
        volScalarField::Internal::New
        (
            "mDot",
            mesh(),
            dimensionedScalar(dimDensity/dimTime, Zero)
        );

    forAll(species(), mDoti)
    {
        tmDot.ref() += mDot(mDoti);
    }

    return tmDot;
}


void Foam::fv::multicomponentPhaseChange::addSup
(
    const volScalarField& alpha,
    const volScalarField& rho,
    const volScalarField& heOrYi,
    fvMatrix<scalar>& eqn
) const
{
    const label i = index(phaseNames(), alpha.group());
    const label s = sign(phaseNames(), alpha.group());

    // Energy equation
    if (index(heNames(), heOrYi.name()) != -1)
    {
        const volScalarField& p = this->p();
        const volScalarField Tchange(vifToVf(this->Tchange()));

        forAll(species(), mDoti)
        {
            const label speciei =
                specieThermos()[i].species()[species()[mDoti]];

            const volScalarField::Internal mDot(this->mDot(mDoti));

            // Direct transfer of energy due to mass transfer
            tmp<volScalarField::Internal> hs =
                vfToVif(specieThermos()[i].hsi(speciei, p, Tchange));
            if (energySemiImplicit_)
            {
                eqn += -fvm::SuSp(-s*mDot, heOrYi) + s*mDot*(hs - heOrYi);
            }
            else
            {
                eqn += s*mDot*hs;
            }

            // Absolute enthalpies at the interface
            Pair<tmp<volScalarField::Internal>> has;
            for (label j = 0; j < 2; ++ j)
            {
                has[j] = vfToVif(specieThermos()[j].hai(speciei, p, Tchange));
            }

            // Latent heat of phase change
            eqn -=
                (i == 0 ? 1 - Lfraction() : Lfraction())
               *mDot*(has.second() - has.first());
        }
    }
    // Mass fraction equation
    else if
    (
        specieThermos().valid()[i]
     && specieThermos()[i].containsSpecie(heOrYi.member())
    )
    {
        // A transferring specie
        if (species().found(heOrYi.member()))
        {
            eqn += s*mDot(species()[heOrYi.member()]);
        }
        // A non-transferring specie. Do nothing.
        else
        {}
    }
    // Something else. Fall back.
    else
    {
        phaseChange::addSup(alpha, rho, eqn);
    }
}


bool Foam::fv::multicomponentPhaseChange::read(const dictionary& dict)
{
    if (phaseChange::read(dict))
    {
        readCoeffs();
        return true;
    }
    else
    {
        return false;
    }
}


// ************************************************************************* //

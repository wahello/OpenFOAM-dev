/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2021-2023 OpenFOAM Foundation
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

Class
    Foam::fvModels

Description
    Finite volume models

SourceFiles
    fvModels.C

\*---------------------------------------------------------------------------*/

#ifndef fvModels_H
#define fvModels_H

#include "fvModel.H"
#include "PtrListDictionary.H"
#include "DemandDrivenMeshObject.H"
#include "HashSet.H"
#include "volFields.H"
#include "geometricOneField.H"
#include "fvMesh.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of friend functions and operators
class fvModels;

Ostream& operator<<(Ostream& os, const fvModels& models);


/*---------------------------------------------------------------------------*\
                           Class fvModels Declaration
\*---------------------------------------------------------------------------*/

class fvModels
:
    public DemandDrivenMeshObject<fvMesh, TopoChangeableMeshObject, fvModels>,
    public dictionary,
    public PtrListDictionary<fvModel>
{
    // Private Member Data

        //- Time index to check that all defined sources have been applied
        mutable label checkTimeIndex_;

        //- Sets of the fields that have had sources added by the fvModels
        mutable PtrList<wordHashSet> addSupFields_;


    // Private Member Functions

        //- Create IO object if dictionary is present
        IOobject createIOobject(const fvMesh& mesh) const;

        //- Check that all fvModels have been applied
        void checkApplied() const;

        //- Return a source for an equation
        template<class Type, class ... AlphaRhoFieldTypes>
        tmp<fvMatrix<Type>> sourceTerm
        (
            const VolField<Type>& eqnField,
            const dimensionSet& ds,
            const AlphaRhoFieldTypes& ... alphaRhoFields
        ) const;



protected:

    friend class DemandDrivenMeshObject
    <
        fvMesh,
        TopoChangeableMeshObject,
        fvModels
    >;

    // Protected Constructors

        //- Construct from components with list of field names
        explicit fvModels(const fvMesh& mesh);


public:

    //- Runtime type information
    TypeName("fvModels");


    // Constructors

        //- Disallow default bitwise copy construction
        fvModels(const fvModels&) = delete;

        //- Inherit the base New method
        using DemandDrivenMeshObject
        <
            fvMesh,
            TopoChangeableMeshObject,
            fvModels
        >::New;


    //- Destructor
    virtual ~fvModels()
    {}


    // Member Functions

        //- Declare fvModels to be a global dictionary
        virtual bool global() const
        {
            return true;
        }


        // Checks

            //- Return true if an fvModel adds a source term to the given
            //  field's transport equation
            virtual bool addsSupToField(const word& fieldName) const;

            //- Return the maximum time-step for stable operation
            virtual scalar maxDeltaT() const;


        // Sources

            //- Correct the fvModels
            //  e.g. solve equations, update model, for film, Lagrangian etc.
            virtual void correct();

            //- Return source for an equation
            template<class Type>
            tmp<fvMatrix<Type>> sourceProxy
            (
                const VolField<Type>& eqnField
            ) const;

            //- Return source for an equation
            template<class Type>
            tmp<fvMatrix<Type>> source
            (
                const VolField<Type>& field
            ) const;

            //- Return source for an equation
            template<class Type>
            tmp<fvMatrix<Type>> sourceProxy
            (
                const VolField<Type>& field,
                const VolField<Type>& eqnField
            ) const;

            //- Return source for a compressible equation
            template<class Type>
            tmp<fvMatrix<Type>> source
            (
                const volScalarField& rho,
                const VolField<Type>& field
            ) const;

            //- Return source for a compressible equation
            template<class Type>
            tmp<fvMatrix<Type>> sourceProxy
            (
                const volScalarField& rho,
                const VolField<Type>& field,
                const VolField<Type>& eqnField
            ) const;

            //- Return source for a phase equation
            template<class Type>
            tmp<fvMatrix<Type>> source
            (
                const volScalarField& alpha,
                const volScalarField& rho,
                const VolField<Type>& field
            ) const;

            //- Return source for a phase equation
            template<class Type>
            tmp<fvMatrix<Type>> sourceProxy
            (
                const volScalarField& alpha,
                const volScalarField& rho,
                const VolField<Type>& field,
                const VolField<Type>& eqnField
            ) const;

            //- Return source for a phase equation
            template<class Type>
            tmp<fvMatrix<Type>> source
            (
                const volScalarField& alpha,
                const geometricOneField& rho,
                const VolField<Type>& field
            ) const;

            //- Return source for a phase equation
            template<class Type>
            tmp<fvMatrix<Type>> source
            (
                const geometricOneField& alpha,
                const volScalarField& rho,
                const VolField<Type>& field
            ) const;

            //- Return source for a phase equation
            template<class Type>
            tmp<fvMatrix<Type>> source
            (
                const geometricOneField& alpha,
                const geometricOneField& rho,
                const VolField<Type>& field
            ) const;

            //- Return source for an equation with a second time derivative
            template<class Type>
            tmp<fvMatrix<Type>> d2dt2
            (
                const VolField<Type>& field
            ) const;


        // Mesh changes

            //- Prepare for mesh update
            virtual void preUpdateMesh();

            //- Update for mesh motion
            virtual bool movePoints();

            //- Update topology using the given map
            virtual void topoChange(const polyTopoChangeMap&);

            //- Update from another mesh using the given map
            virtual void mapMesh(const polyMeshMap&);

            //- Redistribute or update using the given distribution map
            virtual void distribute(const polyDistributionMap&);


        // IO

            //- Read the fvModels dictionary if it has changed
            //  and update the models
            virtual bool read();

            //- ReadData function which reads the fvModels dictionary
            //  required for regIOobject read operation
            virtual bool readData(Istream&);

            //- writeData function required by regIOobject but not used
            //  for this class, writeObject is used instead
            virtual bool writeData(Ostream& os) const;

            //- Write the fvModels
            virtual bool writeObject
            (
                IOstream::streamFormat fmt,
                IOstream::versionNumber ver,
                IOstream::compressionType cmp,
                const bool write
            ) const;


    // Member Operators

        //- Inherit the PtrListDictionary index operators
        using PtrListDictionary<fvModel>::operator[];

        //- Inherit the PtrListDictionary size function
        using PtrListDictionary<fvModel>::size;

        //- Disallow default bitwise assignment
        void operator=(const fvModels&) = delete;


    // IOstream Operators

        //- Ostream operator
        friend Ostream& operator<<
        (
            Ostream& os,
            const fvModels& models
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//- Trait for obtaining global status
template<>
struct typeGlobal<fvModels>
{
    static const bool global = true;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "fvModelsTemplates.C"
#endif


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
